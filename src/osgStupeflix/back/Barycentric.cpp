/*  -*-c++-*- 
 *  Copyright (C) 2010 Stupeflix 
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */

#include <osgStupeflix/Barycentric>


void getBarycentricCoordinate(const osg::Vec2& v , const osg::Vec2& v0, const osg::Vec2& v1, const osg::Vec2& v2, double& y0, double& y1, double& y2)
{
    y0 = (v1[1]-v2[1])*(v[0]-v2[0]) +  (v2[0]-v1[0])*(v[1]-v2[1]) /
        ((v1[1]-v2[1]) * (v0[0]-v2[0]) + (v2[0]-v1[0]) * (v0[1] - v2[1]));

    y1 = (v2[1]-v0[1])*(v[0]-v2[0]) +  (v0[0]-v2[0])*(v[1]-v2[1]) /
        ((v2[1]-v0[1]) * (v1[0]-v2[0]) + (v0[0]-v2[0]) * (v1[1] - v2[1]));
    
    y2 = 1.0 - y1 - y0;
}

// 0---------3
// |         |
// |         |
// 1---------2

 static osg::Vec2 initialUVs[4] = { osg::Vec2(0,1),
                                    osg::Vec2(0,0),
                                    osg::Vec2(1,0),
                                    osg::Vec2(1,1) };

static void convertLocalUVtoGlobalUVOld(const osg::Vec2Array& globalUVs, osg::Vec2Array& localUVs, osg::Vec2Array& resultUVs)
{


    for (int i = 0; i < 4; i++) {

        osg::Vec2 newUV;
        double y0, y1, y2; // barycentric coordinate
        getBarycentricCoordinate(localUVs[i], initialUVs[0], initialUVs[1], initialUVs[2],
                                 y0, y1, y2);
    
        if (y0 < 0.0 || y0 > 1.0 ||
            y1 < 0.0 || y1 > 1.0 ||
            y2 < 0.0 || y2 > 1.0) {

            getBarycentricCoordinate(localUVs[i], initialUVs[2], initialUVs[3], initialUVs[0],
                                     y0, y1, y2);
            newUV = globalUVs[2] * y0 + globalUVs[3]*y1 + globalUVs[0] * y2;
        } else {
            newUV = globalUVs[0] * y0 + globalUVs[1]*y1 + globalUVs[2] * y2;
        }

        resultUVs[i] = newUV;
    }
}


void convertLocalUVtoGlobalUV(const osg::Vec2Array& globalUVs, osg::Vec2Array& localUVs, osg::Vec2Array& resultUVs)
{

    // generate one axis
    osg::Vec2 o = localUVs[0];
    osg::Vec2 v0 = localUVs[3]-localUVs[0];
    osg::Vec2 v1 = localUVs[1]-localUVs[0];

    for (int i = 0; i < 4; i++) {
        osg::Vec2 newUV;
        newUV = o;
        newUV += v0 * globalUVs[i][0];
        newUV += v1 * globalUVs[i][1];
        resultUVs[i] = newUV;
    }
}
