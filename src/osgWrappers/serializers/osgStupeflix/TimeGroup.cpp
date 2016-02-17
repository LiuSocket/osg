#include <osgStupeflix/TimeGroup>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgStupeflix_TimeGroup,
                         new osgStupeflix::TimeGroup,
                         osgStupeflix::TimeGroup,
                         "osg::Object osg::Node osg::Group osgStupeflix::TimeGroup" )
{
    ADD_DOUBLE_SERIALIZER( Duration, 1.0 );
    ADD_DOUBLE_SERIALIZER( StartTime, 0.0 );
}
