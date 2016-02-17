/*  -*-c++-*-
 *  Copyright (C) 2012 Stupeflix
 *
 * Authors:
 *        François Lagunas <francois.lagunas@gmail.com>
 */

#include <osgStupeflix/RenderOnce>
#include <osgUtil/CullVisitor>


using namespace osgStupeflix;
#define RENDERONCE_DEBUG 0

osgStupeflix::RenderOnce::RenderOnce(const RenderOnce& b, const osg::CopyOp& copyop) : osg::Group(b, copyop), _dirty(b._dirty) {}

class CullCallback : public osg::NodeCallback
{
  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
      osgStupeflix::RenderOnce* renderOnce = dynamic_cast<osgStupeflix::RenderOnce*>(node);
      float simulationTime = nv->getFrameStamp()->getSimulationTime();
      traverse(node, nv);
      renderOnce->wasDrawn(simulationTime);
  }
};

osgStupeflix::RenderOnce::RenderOnce() : osg::Group()
{
#if RENDERONCE_DEBUG
    osg::notify(osg::INFO) << "RenderOnce create" << std::endl;
#endif
    _dirty = true;
    setCullCallback(new CullCallback());
}

void osgStupeflix::RenderOnce::reset() {
#if RENDERONCE_DEBUG
    osg::notify(osg::INFO) << "RenderOnce reset" << std::endl;
#endif
    _dirty = true;
}

osg::Node::NodeMask osgStupeflix::RenderOnce::nodeMask(float time) {
#if RENDERONCE_DEBUG
    osg::notify(osg::INFO) << "RenderOnce nodeMask at " << time << " : " << _dirty << std::endl;
#endif
    if (_dirty) {
        return ACTIVE | PREFETCH;
    } else {
        return 0;
    }
}

bool osgStupeflix::RenderOnce::needCleaning(float time) {
#if RENDERONCE_DEBUG
    osg::notify(osg::INFO) << "RenderOnce needCleaning at " << time << " : " << ! _dirty << std::endl;
#endif
    return ! _dirty;
}

void osgStupeflix::RenderOnce::wasDrawn(float time) {
#if RENDERONCE_DEBUG
    osg::notify(osg::INFO) << "RenderOnce wasDrawn" << std::endl;
#endif
    _dirty = false;
}
