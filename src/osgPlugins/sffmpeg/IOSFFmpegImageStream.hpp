
#ifndef HEADER_GUARD_OSGFFMPEG_IOSFFMPEG_IMAGE_STREAM_H
#define HEADER_GUARD_OSGFFMPEG_IOSFFMPEG_IMAGE_STREAM_H
#import <Foundation/Foundation.h>
#include "FFmpegImageStream.hpp"

namespace osgFFmpeg
{

    class IOSFFmpegImageStream : public FFmpegImageStream
    {
    public:
        IOSFFmpegImageStream();
        IOSFFmpegImageStream(const IOSFFmpegImageStream & image, const osg::CopyOp & copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgFFmpeg, IOSFFmpegImageStream);

        void quit(bool /*waitForThreadToExit*/);

    protected:
        void open_finish();
        const char* get_real_file_name();
        NSMutableDictionary* _stub;
    };

}

#endif // HEADER_GUARD_OSGFFMPEG_FFMPEG_IMAGE_STREAM_H
