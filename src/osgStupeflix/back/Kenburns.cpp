/*  -*-c++-*- 
 *  Copyright (C) 2010 Stupeflix 
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/Kenburns>
#include <osgStupeflix/Barycentric>

using namespace osgStupeflix;


Kenburns::Kenburns(): DrawUpdateCallback()
{
    _cornerUpLeft = osg::Vec2(0.0,0.0);
    _cornerBottomRight = osg::Vec2(1.0,1.0);
}

// 0---------3
// |         |
// |         |
// 1---------2

void Kenburns::update(osg::NodeVisitor* nv, osg::Drawable* drw, osg::Vec2Array* source)
{
    if (!source)
        return;

    osg::Vec2Array* arraySrc = source;
    osg::Vec2Array* arrayDst = _uvs;
    if (!arrayDst || !arraySrc) {
        osg::notify(osg::WARN) << "osgStupeflix: A source or Destination dont have uvs to make a Kenburns" << std::endl;
        return;
    }
    
    osg::Vec2 translate (1,0);
    
    double t = nv->getFrameStamp()->getSimulationTime();
    t = t/10.0;

    double t1 = 1.0 - t;
    osg::ref_ptr<osg::Vec2Array> currentUVs = new osg::Vec2Array(4);
    osg::Vec2Array& uvs = *currentUVs;
    uvs[0] = _cornerUpLeft;
    uvs[1] = osg::Vec2(_cornerUpLeft[0],_cornerBottomRight[1]);
    uvs[2] = _cornerBottomRight;
    uvs[3] = osg::Vec2(_cornerBottomRight[0],_cornerUpLeft[1]);

    for (int i = 0; i < 4; i++) {
        uvs[i] += osg::Vec2(0.01*t,0);
    }

    convertLocalUVtoGlobalUV(*source, uvs, *arrayDst);

    arrayDst->dirty();
    DrawUpdateCallback::update(nv, drw, getUVArray());
}
