
#ifndef HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
#define HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H

#include <osgStupeflix/ImageStream>
#include "FFmpegParameters.hpp"

extern "C" {
#include "sxplayer.h"
}

namespace osgFFmpeg
{

    class FFmpegImageStream : public osgStupeflix::ImageStream
    {
    public:

        FFmpegImageStream();
        FFmpegImageStream(const FFmpegImageStream & image, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgFFmpeg, FFmpegImageStream);

        bool open(const std::string & filename, FFmpegParameters* parameters);

        void play();

        void update(osg::NodeVisitor* nv);
        bool requiresUpdateCall() const { return true; }

        virtual void setFrameTimeChangedCallback(osgStupeflix::FrameTimeChangedCallback callback, void* callbackData);
        virtual void quit(bool /*waitForThreadToExit*/);

        virtual bool ready(double time);

    protected:

        struct sxplayer_ctx *_video;
        struct sxplayer_frame *_last_frame;

        bool _av_select;
        bool _playEvent;
        double _start_time;
        double _trim_duration;
        double _skip;
        bool _opened;
        bool _blackFrameSet; // is true, the last outputed data is a black transparent frame, instead of last frame. Used to handle some cases in seeking.

        virtual ~FFmpegImageStream();
        virtual void open_finish() {}
        bool reader_open();
        void reader_release_last_frame();
        void reader_close();
        void resetImageStreamToBlack();
        // Reuse must be set to true if the frame was already passed to setImageFromFrame
        void setImageFromFrame(sxplayer_frame *frame, double relativeTime, bool reuse);

        virtual const char* get_real_file_name() {
            return _fileName.c_str();
        }

        osgStupeflix::FrameTimeChangedCallback _frameTimeChangedCallback;
        void* _frameTimeChangedCallbackData;


    };

}

#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
