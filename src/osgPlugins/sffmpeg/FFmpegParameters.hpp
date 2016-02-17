#ifndef SFFMPEG_PARAMETERS_H
#define SFFMPEG_PARAMETERS_H

#include <osg/Referenced>
#include <sstream>

namespace osgFFmpeg {

#define SELECT_VIDEO 0
#define SELECT_AUDIO 1

class FFmpegParameters : public osg::Referenced {
    public:

  FFmpegParameters() : avselect(SELECT_VIDEO), start_time(0), skip(0), trim_duration(-1) {}
        ~FFmpegParameters() {}

        void parse(const std::string& name, const std::string& value) {
            std::stringstream ss(value);

            if (value.empty()) {
                return;
            } else if (name == "avselect") {
                if (value == "audio")
                    avselect = SELECT_AUDIO;
                else if (value == "video")
                    avselect = SELECT_VIDEO;
            } else if (name == "start_time") {
                ss >> start_time;
            } else if (name == "skip_time") {
                ss >> skip;
            } else if (name == "trim_duration") {
                ss >> trim_duration;
            }
        }

        int avselect;
        double start_time;
        double skip;
        double trim_duration;
    };
}

#endif
