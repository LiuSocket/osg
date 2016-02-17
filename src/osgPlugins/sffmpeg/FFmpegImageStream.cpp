#include "FFmpegImageStream.hpp"
#include <osg/ImageUtils>
#include <osg/Notify>
#include <osg/NodeVisitor>
#include <cstdio>
#include <memory>

#if defined(__APPLE__)

#include <VideoToolbox/VideoToolbox.h>
#endif
#if READER_PLATFORM_IOS
#include <mach/mach_time.h>
#include <Foundation/Foundation.h>

uint64_t timeGet() {
    return  mach_absolute_time();
}



float logTimeDiff(NSString* message, uint64_t start, uint64_t end) {
    mach_timebase_info_data_t timeBaseInfo;
    mach_timebase_info(&timeBaseInfo);

    uint64_t elapsedTime = end - start;
    uint64_t elapsedTimeNano = elapsedTime * timeBaseInfo.numer / timeBaseInfo.denom + 1;

    double elapsed = float(elapsedTimeNano) / 1.0E9;
    if (elapsed >= 0.05) {
        NSLog(@"%@, time= %f", message, elapsed);
    }

    return elapsed;
}

#endif

static unsigned char blackTexture[4];

namespace osgFFmpeg {

FFmpegImageStream::FFmpegImageStream() :
    osgStupeflix::ImageStream(),
    _video(NULL),
    _av_select(true),
    _playEvent(false),
    _frameTimeChangedCallback(NULL),
    _frameTimeChangedCallbackData(NULL),
    _last_frame(NULL),
    _opened(false),
    _blackFrameSet(false)
{
    setOrigin(osg::Image::TOP_LEFT);
    _playEvent = false;
}

FFmpegImageStream::FFmpegImageStream(const FFmpegImageStream & image, const osg::CopyOp & copyop) :
     osgStupeflix::ImageStream(image, copyop),
    _video(NULL),
    _playEvent(false),
    _frameTimeChangedCallback(NULL),
    _frameTimeChangedCallbackData(NULL),
    _last_frame(NULL),
    _opened(false)
{
    setOrigin(osg::Image::TOP_LEFT);
    // TODO: probably incorrect or incomplete

    _playEvent = false;
}

FFmpegImageStream::~FFmpegImageStream()
{
    OSG_INFO<<"Destructing FFmpegImageStream..."<<std::endl;
    // Callback to let know we are getting destroyed
    if (_frameTimeChangedCallback != NULL)
        _frameTimeChangedCallback(osgStupeflix::DESTRUCTION_TIME, _frameTimeChangedCallbackData);

    reader_close();
}

void FFmpegImageStream::resetImageStreamToBlack() {
    if (_blackFrameSet) {
        return;
    }

#if defined(ANDROID)
    GLint format = GL_RGBA;
#else
    GLint format = GL_BGRA;
#endif
    GLint internalFormat = GL_RGBA;
    // Reset the image to a black transparent one
    memset(blackTexture, 0, sizeof(blackTexture));
    setImage(1, 1, 1,
             internalFormat, format, GL_UNSIGNED_BYTE, blackTexture, NO_DELETE,
             1, 1);
    _blackFrameSet = true;
}


bool FFmpegImageStream::open(const std::string & filename, FFmpegParameters* parameters)
{
    _av_select = parameters->avselect;
    _referenceTime = parameters->start_time;
    _trim_duration = parameters->trim_duration;
    _start_time = parameters->start_time;
    _fileName = filename;
    _skip = parameters->skip;

    open_finish();
    resetImageStreamToBlack();
    return true;
}

void FFmpegImageStream::play() { _status=PLAYING; _playEvent = true; }

bool FFmpegImageStream::reader_open() {
    if (!_opened) {
        const char* filename = get_real_file_name();
        if (!filename) {
            return false;
        }

        char buf[256], *filters = NULL;

#if defined(ANDROID)
        int format = SXPLAYER_PIXFMT_RGBA;
#else
        int format = SXPLAYER_PIXFMT_BGRA;
#endif
        _video = sxplayer_create(filename);

        if (!_video) {
            return false;
        }

        sxplayer_set_option(_video, "avselect", _av_select);
        sxplayer_set_option(_video, "skip", _skip);
        sxplayer_set_option(_video, "trim_duration", _trim_duration);

#if defined(ANDROID) || READER_PLATFORM_IOS
        sxplayer_set_option(_video, "max_nb_packets", 1);
        sxplayer_set_option(_video, "max_nb_frames",  1);
        sxplayer_set_option(_video, "max_nb_sink",    1);
#endif

        if (filters) {
            sxplayer_set_option(_video, "filters", filters);
            sxplayer_set_option(_video, "auto_hwaccel", 0);
        }

        sxplayer_set_option(_video, "sw_pix_fmt", format);

        _opened = true;
    }

    return true;
}

void FFmpegImageStream::reader_release_last_frame() {
    if (_last_frame) {
#if defined(__APPLE__)
        if (_last_frame->pix_fmt == SXPLAYER_PIXFMT_VT) {
            CVPixelBufferRef pixbuf = (CVPixelBufferRef)_last_frame->data;
            CVPixelBufferUnlockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
        }
#endif
        sxplayer_release_frame(_last_frame);
        _last_frame = NULL;
    }

}

void FFmpegImageStream::reader_close() {
    //    printf("FFmpegImageStream::reader_close\n");
    reader_release_last_frame();
    sxplayer_free(&_video);
    _opened = false;
}

  void FFmpegImageStream::setImageFromFrame(sxplayer_frame *frame, double relativeTime, bool reuse) {
    // It seems internalformat and pixelformat should be the same on Android
    GLint internalFormat = GL_RGBA;

    uint8_t *data = frame->data;
    int linesize = frame->linesize;
#if defined(ANDROID)
    GLint vbufpixfmt = GL_RGBA;
#else
    GLint vbufpixfmt = GL_BGRA;
#endif

#if defined(__APPLE__)
    if (frame->pix_fmt == SXPLAYER_PIXFMT_VT) {
        CVPixelBufferRef pixbuf = (CVPixelBufferRef)frame->data;

        if (!reuse) {
            CVReturn err = CVPixelBufferLockBaseAddress(pixbuf, kCVPixelBufferLock_ReadOnly);
            if (err != kCVReturnSuccess) {
                OSG_WARN << "Unable to lock pixel buffer" << std::endl;
                return;
            }
        }

        if (CVPixelBufferIsPlanar(pixbuf)) {
            // XXX: just pick luma waiting for sth better (debug code path for
            // pixbuf such as nv12)
            vbufpixfmt = internalFormat = GL_LUMINANCE;
            data       = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixbuf, 0);
            linesize   = CVPixelBufferGetBytesPerRowOfPlane(pixbuf, 0) << 2;
        } else {
            data     = (uint8_t*)CVPixelBufferGetBaseAddress(pixbuf);
            linesize = CVPixelBufferGetBytesPerRow(pixbuf);
        }
    }
#endif

    linesize >>= 2;

    if (_av_select == SELECT_VIDEO)
        setImage(frame->width, frame->height, 1,
                 internalFormat, vbufpixfmt, GL_UNSIGNED_BYTE, data, NO_DELETE,
                 1, linesize);
#if !defined(ANDROID) && ! READER_PLATFORM_IOS
    else
        setImage(frame->width, frame->height, 1,
                 GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT, data, NO_DELETE,
                 1, linesize);
#endif
    _blackFrameSet = false;
    updateClients(data, frame->width, frame->height, relativeTime);
}

void FFmpegImageStream::update(osg::NodeVisitor* nv)
{
    open_finish();
    bool ok = reader_open();
    if (!ok) {
//        printf("FFmpegImageStream::update : reader open failed\n");
        return;
    }

    if (_playEvent) {
        _referenceTime = nv->getFrameStamp()->getSimulationTime();
        _playEvent = false;
    }

    if (_status == PAUSED) {
        return;
    }

    double absTime = nv->getFrameStamp()->getSimulationTime();
    double t = -1.0;

    if (absTime >= _start_time) {
        t = getRemappedTime(absTime);
    } else {
        resetImageStreamToBlack();
        sxplayer_prefetch(_video);
        return;
    }

    //    NSLog(@"getting frame %f %f => %f", absTime, _start_time, t);
    //    uint64_t a = timeGet();
    //    NSLog(@"video reading start %s", _fileName.c_str());
    struct sxplayer_frame *frame = sxplayer_get_frame(_video, t);
    //    uint64_t b = timeGet();

    //    float elapsed = logTimeDiff([NSString stringWithFormat:@"video reading end %s", _fileName.c_str()], a,b);

    if (!frame){
        //        printf("FFmpegImageStream::update : read frame failed\n");
        //        printf("get frame failed %s\n", _fileName.c_str());
        if (_blackFrameSet && _last_frame) {
           //           printf("No new frame, reset to last frame\n");
           setImageFromFrame(_last_frame, t, true);
        }
        return;
    } else {
        //        printf("FFmpegImageStream::update : read frame ok\n");
        //        printf("get frame ok %p, %s, %d , %d\n", this, _fileName.c_str(), frame->width, frame->height);
    }

    // TEMPORARY: TODO on slomo videos
    //sxplayer_set_drop_ref(_video, 1);

    // Reset the flag
    setImageFromFrame(frame, t, false);

    reader_release_last_frame();
    _last_frame = frame;

    if (_frameTimeChangedCallback != NULL)
        _frameTimeChangedCallback(t, _frameTimeChangedCallbackData);
}

void FFmpegImageStream::setFrameTimeChangedCallback(osgStupeflix::FrameTimeChangedCallback callback, void* callbackData)
{
    _frameTimeChangedCallback = callback;
    _frameTimeChangedCallbackData = callbackData;
}

void FFmpegImageStream::quit(bool waitForThreadToExit) {
    //printf("FFmpegImageStream::quit: resetImageStreamToBlack\n");
    resetImageStreamToBlack();
    reader_close();
}

bool FFmpegImageStream::ready(double time) {
    // TEMPORARY : need locking ?
    if (time < _start_time) {
        return true;
    }
    return _last_frame != NULL;
}

} // namespace osgFFmpeg

