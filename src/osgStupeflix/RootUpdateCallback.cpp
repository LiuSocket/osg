/*  -*-c++-*-
 *  Copyright (C) 2010 Stupeflix
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osg/Texture>
#include <osgStupeflix/MarkActiveNodeVisitor>
#include <osgStupeflix/RootUpdateCallback>
#include <osgStupeflix/ImageCleanVisitor>

using namespace osgStupeflix;


RootUpdateCallback::RootUpdateCallback(bool debug):
  osg::NodeCallback(),
  debug(debug)
{
}

void RootUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    const osg::FrameStamp* fs = nv->getFrameStamp();

//    osg::notify(osg::INFO) << "RootUpdateCallback" << fs << std::endl;

    if (fs) {
        double t = fs->getSimulationTime();
        //osg::notify(osg::INFO) << " Time is " << t << std::endl;

    }

    traverse(node,nv);
}
