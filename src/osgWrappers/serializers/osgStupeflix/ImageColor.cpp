#include <osgStupeflix/ImageColor>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgStupeflix_ImageColor,
                         new osgStupeflix::ImageColor,
                         osgStupeflix::ImageColor,
                         "osg::Object osg::Image osgStupeflix::ImageColor" )
{
    ADD_UINT_SERIALIZER( R, 0 );
    ADD_UINT_SERIALIZER( G, 0 );
    ADD_UINT_SERIALIZER( B, 0 );
    ADD_UINT_SERIALIZER( A, 0 );
}
