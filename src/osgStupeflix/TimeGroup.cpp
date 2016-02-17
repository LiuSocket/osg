/*  -*-c++-*- 
 *  Copyright (C) 2010 Stupeflix 
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */

#include <osgStupeflix/TimeGroup>

using namespace osgStupeflix;

osgStupeflix::TimeGroup::TimeGroup(const TimeGroup& b, const osg::CopyOp& copyop) : osg::Group(b, copyop), _start(b._start), _duration(b._duration) {}

osgStupeflix::TimeGroup::TimeGroup() : osg::Group()
{
    _start = 0;
    _duration = 1.0;
}


