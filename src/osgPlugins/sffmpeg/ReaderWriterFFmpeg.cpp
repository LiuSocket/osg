/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "FFmpegParameters.hpp"

#if defined(READER_PLATFORM_IOS)
  #include "IOSFFmpegImageStream.hpp"
#else
  #include "FFmpegImageStream.hpp"
#endif

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

/** Implementation heavily inspired by http://www.dranger.com/ffmpeg/ */

class ReaderWriterFFmpeg : public osgDB::ReaderWriter
{
public:

    ReaderWriterFFmpeg()
    {
        supportsProtocol("http","Read video/audio from http using ffmpeg.");
        supportsProtocol("rtsp","Read video/audio from rtsp using ffmpeg.");

        // XXX: Keep this list in sync with src/osgDB/Registry.cpp...
        supportsExtension("sffmpeg", "");
        supportsExtension("avi",    "Audio Video Interleaved");
        supportsExtension("flv",    "Flash video");
        supportsExtension("mov",    "Quicktime");
        supportsExtension("ogg",    "Theora movie format");
        supportsExtension("ogv",    "Theora movie format");
        supportsExtension("mpg",    "Mpeg movie format");
        supportsExtension("mpv",    "Mpeg movie format");
        supportsExtension("wmv",    "Windows Media Video format");
        supportsExtension("mkv",    "Matroska");
        supportsExtension("webm",   "WebM");
        supportsExtension("mjpeg",  "Motion JPEG");
        supportsExtension("mp4",    "MPEG-4");
        supportsExtension("dv",     "DV (Digital video)");
        supportsExtension("sav",    "Unknown");
        supportsExtension("3gp",    "3G multi-media format");
        supportsExtension("sdp",    "Session Description Protocol");
        supportsExtension("m2ts",   "MPEG-2 Transport Stream");
        supportsExtension("mp3",    "MP3 (MPEG audio layer 3)");
        supportsExtension("flac",   "FLAC (Free Lossless Audio Codec)");
        supportsExtension("wav",    "WAV (Waveform Audio)");
        supportsExtension("nut",    "NUT container format");
        supportsExtension("jpg",   "jpg image file");
        supportsExtension("jpeg",  "jpeg image file");
        supportsExtension("jpe",   "jpe image file");
        supportsExtension("jp2",   "jp2 image file");
        supportsExtension("tiff",  "tiff image file");
        supportsExtension("tif",   "tif image file");
        supportsExtension("gif",   "gif image file");
        supportsExtension("png",   "png image file");

        supportsOption("avselect", "pick type");
        supportsOption("visible_time",  "time when the first video frame is decoded");
        supportsOption("start_time",    "time when the playback actually starts");
        supportsOption("skip_time",     "initial seek in the video");
        supportsOption("trim_duration", "duration of the video, relative to skip");
        supportsOption("speed",         "playback speed");
        supportsOption("type",          "media type (image or video)");
    }

    virtual ~ReaderWriterFFmpeg()
    {
    }

    virtual const char * className() const
    {
        return "ReaderWriterSFFmpeg";
    }

    virtual ReadResult readImage(const std::string & filename, const osgDB::ReaderWriter::Options* options) const
    {
        const std::string ext = osgDB::getLowerCaseFileExtension(filename);
        if (ext=="sffmpeg")
            return readImage(osgDB::getNameLessExtension(filename),options);

        if (filename.compare(0, 5, "/dev/")==0)
        {
            return readImageStreamSync(filename, NULL);
        }

        osg::ref_ptr<osgFFmpeg::FFmpegParameters> parameters(new osgFFmpeg::FFmpegParameters);
        parseOptions(parameters.get(), options);

        //        if (! acceptsExtension(ext))
        //            return ReadResult::FILE_NOT_HANDLED;

        //        const std::string path = osgDB::containsServerAddress(filename) ?
        //            filename :
        //            osgDB::findDataFile(filename, options);
        const std::string path = filename;

        if (path.empty())
            return ReadResult::FILE_NOT_FOUND;

        return readImageStreamSync(filename, parameters.get());
    }

    ReadResult readImageStreamSync(const std::string& filename, osgFFmpeg::FFmpegParameters* parameters) const
    {
        OSG_INFO << "ReaderWriterFFmpeg::readImage " << filename << std::endl;

#if READER_PLATFORM_IOS
        osg::ref_ptr<osgFFmpeg::IOSFFmpegImageStream> image_stream(new osgFFmpeg::IOSFFmpegImageStream);
#else
        osg::ref_ptr<osgFFmpeg::FFmpegImageStream> image_stream(new osgFFmpeg::FFmpegImageStream);
#endif

        if (! image_stream->open(filename, parameters))
            return ReadResult::FILE_NOT_HANDLED;

        return image_stream.release();
    }

private:
    void parseOptions(osgFFmpeg::FFmpegParameters* parameters, const osgDB::ReaderWriter::Options * options) const
    {
        if (options && options->getNumPluginStringData()>0)
        {
            const FormatDescriptionMap& supportedOptList = supportedOptions();
            for (FormatDescriptionMap::const_iterator itr = supportedOptList.begin();
                 itr != supportedOptList.end(); ++itr)
            {
                const std::string& name = itr->first;
                parameters->parse(name, options->getPluginStringData(name));
            }
        }
    }
};

REGISTER_OSGPLUGIN(sffmpeg, ReaderWriterFFmpeg)
