#include <iostream>
#include <osgStupeflix/OpenCvTracker>

#if !TRACKING_NOT_SUPPORTED

using namespace osgStupeflix;

OpenCvTracker::OpenCvTracker(osg::Vec2 const &center, osg::Vec2 const &size,
                             double startTime, double endTime)
: Tracker(center, size, startTime, endTime),
  _tracker_base(cv::Vec2(center.x(), center.y()), cv::Vec2(size.x(), size.y()))
{}

osg::Vec2 OpenCvTracker::doMatching(const unsigned char *frameData, unsigned int width,
                                       unsigned int height)
{
    // Call the matching template
    _tracker_base.doMatching(frameData, width, height);
    // Get the new position of the template
    cv::Vec2 new_center = _tracker_base.center();
    return osg::Vec2(new_center.x, new_center.y);
}

OpenCvTracker::~OpenCvTracker()
{
}

#endif
