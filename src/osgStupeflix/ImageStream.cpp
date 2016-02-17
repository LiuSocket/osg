/* -*-c++-*- OpenSceneGraph - Copyright (C) 2011 Stupeflix

*/

#include <osgStupeflix/ImageStream>
#include <osg/Notify>

namespace osgStupeflix {


  void setVideoParams(osg::Image* image, double localOffset, double speed)
  {
      osg::ImageStream* is = dynamic_cast<osg::ImageStream*>(image);
      if (!image) {
          osg::notify(osg::WARN) << "osgStupeflix: can't set video params, no ImageStream found"<< std::endl;
          return;
      }
      is->seek(localOffset);
      is->setTimeMultiplier(speed);
      is->play();
  }

  void setVideoTimeMap(osg::Image* image, osgAnimation::FloatVariableSpeedCubicBezierKeyframeContainer* container) {
      osgStupeflix::ImageStream* is = dynamic_cast<osgStupeflix::ImageStream*>(image);
      if (!is) {
          osg::notify(osg::WARN) << "osgStupeflix: can't set video params, no ImageStream found"<< std::endl;
          return;
      }
      osgAnimation::FloatVariableSpeedCubicBezierSampler* sampler = new osgAnimation::FloatVariableSpeedCubicBezierSampler();
      sampler->setKeyframeContainer(container);
      is->setVideoTimeMap(sampler);
  }

  void setSamplerKeyFrameContainer(osgAnimation::FloatVariableSpeedCubicBezierSampler* sampler, osgAnimation::FloatVariableSpeedCubicBezierKeyframeContainer* container) {
      sampler->setKeyframeContainer(container);
  }

  float getSamplerValueAt(osgAnimation::FloatVariableSpeedCubicBezierSampler* sampler, float time) {
      float value;
      sampler->getValueAt(time, value);
      return value;
  }

  bool realTimeFlag = true;

  bool realTimeVideoModeGet() {
      return realTimeFlag;
  }
 
  void realTimeVideoModeSet(bool rTime) {
      realTimeFlag = rTime;
  }
}
