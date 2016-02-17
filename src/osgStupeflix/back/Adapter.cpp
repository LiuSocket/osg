/*  -*-c++-*- 
 *  Copyright (C) 2010 Stupeflix 
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/Adapter>
#include <osgStupeflix/Barycentric>
#include <osg/io_utils>
#include <osg/Notify>

using namespace osgStupeflix;


Adapter::Adapter(): DrawUpdateCallback()
{
}

void Adapter::update(osg::NodeVisitor* nv, osg::Drawable* drw, osg::Vec2Array* targetUVs, int unit)
{
    if (!targetUVs)
        return;

    double t = nv->getFrameStamp()->getSimulationTime();
    double ratio = 0;
    if (t < _start)
        ratio = 0;
    else if (t >= (_start +_duration))
        ratio = 1.0;
    else {
        ratio = (t-_start) / _duration;
    }

    interpolate(ratio , *_source, *_target, *getUVArray());

    osg::notify(osg::INFO) << "osgStupeflix: Adapter " << t << " " << this << " current local " << (*getUVArray())[0] << " " << (*getUVArray())[1] << " " << (*getUVArray())[2] << " " << (*getUVArray())[3] << std::endl;

    convertLocalUVtoGlobalUV(*targetUVs, *getUVArray(), *getUVArray());
    osg::notify(osg::INFO) << "osgStupeflix: Adapter " << t << " " << this << " global local " << (*getUVArray())[0] << " " << (*getUVArray())[1] << " " << (*getUVArray())[2] << " " << (*getUVArray())[3] << std::endl;

    osg::notify(osg::INFO) << "osgStupeflix: Adapter " << t << " " << this << " src " << (*_source)[0] << " " << (*_source)[1] << " " << (*_source)[2] << " " << (*_source)[3] << std::endl;
    osg::notify(osg::INFO) << "osgStupeflix: Adapter " << t << " " << this << " dst " << (*_target)[0] << " " << (*_target)[1] << " " << (*_target)[2] << " " << (*_target)[3] << std::endl;

    DrawUpdateCallback::update(nv, drw, getUVArray(), unit);
}
