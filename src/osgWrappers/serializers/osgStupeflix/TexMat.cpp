#include <osgStupeflix/TexMat>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgStupeflix_TexMat,
                         new osgStupeflix::TexMat,
                         osgStupeflix::TexMat,
                         "osg::Object osg::StateAttribute osg::TexMat osgStupeflix::TexMat" )
{
    ADD_UINT_SERIALIZER( Index, 0);
}
