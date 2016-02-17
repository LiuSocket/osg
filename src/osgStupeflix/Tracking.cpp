#include <iostream>
#include <assert.h>
#include <osgStupeflix/Tracking>

using namespace osgStupeflix;

#if !TRACKING_NOT_SUPPORTED

#include <osgStupeflix/OpenCvTracker>

void Tracking::newFrame(const unsigned char* data, unsigned int width, unsigned int height, double time)
{
  // Pass the new frame to trackers
  for (unsigned i = 0; i < this->_trackers.size(); ++i)
    if (this->_trackers[i]->startTime() <= time && time <= this->_trackers[i]->endTime())
      this->_trackers[i]->match(data, width, height, time);
}

void Tracking::addTracker(osg::ref_ptr<Tracker> const &tracker)
{
  // Push the tracker into the vector
  this->_trackers.push_back(tracker);
}

void Tracking::reset() {
    // Clear the vector
    this->_trackers.clear();
}

Tracking::Tracking() : _trackers() {}

Tracking::~Tracking()
{
    // delete trackers
    this->_trackers.clear();
}

#else // if TRACKING_NOT_SUPPORTED => set an empty tracking object

void Tracking::newFrame(const unsigned char* data, unsigned int width, unsigned int height, double time)
{
}

void Tracking::addTracker(osg::ref_ptr<Tracker> const &tracker)
{
}

void Tracking::reset() {
}

Tracking::Tracking() : _trackers()
{
    std::cerr << "Tracking not supported" << std::endl;
}

Tracking::~Tracking()
{
}


#endif
