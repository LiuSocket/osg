/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <string.h>
#include <osgStupeflix/ImageColor>
#include <cstring>

using namespace osgStupeflix;

osgStupeflix::ImageColor::ImageColor(const ImageColor& b, const osg::CopyOp& copyop) : osg::Image(b, copyop) {
    allocateImage(1,1,1,GL_RGBA,GL_UNSIGNED_BYTE,1);
    std::memcpy(data(), b.data(), 4);
}


osgStupeflix::ImageColor::ImageColor() : osg::Image()
{
    allocateImage(1,1,1,GL_RGBA,GL_UNSIGNED_BYTE,1);
}

osgStupeflix::ImageColor::ImageColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a): osg::Image()
{
    allocateImage(1,1,1,GL_RGBA,GL_UNSIGNED_BYTE,1);
    data()[0] = r;
    data()[1] = g;
    data()[2] = b;
    data()[3] = a;
}



osg::Image* osgStupeflix::createImageFromColor(int r, int g, int b, int a)
{
    osgStupeflix::ImageColor* image = new osgStupeflix::ImageColor(r,g,b,a);

    return image;
}
