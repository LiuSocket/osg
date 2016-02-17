/*  -*-c++-*- 
 *  Copyright (C) 2010 Stupeflix 
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/Filters>
#include <osgStupeflix/Adapter>

using namespace osgStupeflix;

void osgStupeflix::addAdapter(double start,
                              double duration,
                              int unit,
                              osg::Geometry* target,
                              osg::Vec2Array* uvSource,
                              osg::Vec2Array* uvTarget)
{
    Adapter* p = new Adapter;
    p->setStartTime(start);
    p->setDuration(duration);
    p->setSource(uvSource);
    p->setTarget(uvTarget);

    MultiDrawUpdateCallback* cb = dynamic_cast<MultiDrawUpdateCallback*>(target->getUpdateCallback());
    if (!cb) {
        cb = new MultiDrawUpdateCallback;
        target->setUpdateCallback(cb);
        //cb->cloneUV(target);
    }
    cb->addUpdateCallback(unit, p);
}
