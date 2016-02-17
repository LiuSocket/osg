#include <osg/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( ImageUpdateCallback,
                         new osg::ImageUpdateCallback,
                         osg::ImageUpdateCallback,
                         "osg::Object osg::ImageUpdateCallback" )
{
}
