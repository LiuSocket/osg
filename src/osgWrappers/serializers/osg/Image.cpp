#include <osg/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Image,
                         new osg::Image,
                         osg::Image,
                         "osg::Object osg::Image" )
{
    // Everything is done in OutputStream and InputStream classes
    ADD_STRING_SERIALIZER( FileName, "" ); 

    BEGIN_ENUM_SERIALIZER2( WriteHint, osg::Image::WriteHint, EXTERNAL_FILE );
        ADD_ENUM_VALUE( NO_PREFERENCE );
        ADD_ENUM_VALUE( STORE_INLINE );
        ADD_ENUM_VALUE( EXTERNAL_FILE );
    END_ENUM_SERIALIZER();  // _image

 }

