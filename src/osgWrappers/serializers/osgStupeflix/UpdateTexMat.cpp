#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


static bool checkTransforms( const osgStupeflix::UpdateTexMat& texMat )
{
   osgAnimation::StackedTransform transforms = texMat.getTransforms();
   return transforms.size() > 0;
}

static bool readTransforms( osgDB::InputStream& is, osgStupeflix::UpdateTexMat& texMat )
{
    osgAnimation::StackedTransform& transforms = texMat.getTransforms();
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgAnimation::StackedTransformElement* child = dynamic_cast<osgAnimation::StackedTransformElement*>( is.readObject() );
        if ( child ) {
            transforms.push_back(child );
        }
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeTransforms( osgDB::OutputStream& os, const osgStupeflix::UpdateTexMat& texMat )
{
    const osgAnimation::StackedTransform& transforms = texMat.getTransforms();

    unsigned int size = transforms.size();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os.writeObject(transforms[i]);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}


REGISTER_OBJECT_WRAPPER( osgStupeflix_UpdateTexMat,
                         new osgStupeflix::UpdateTexMat,
                         osgStupeflix::UpdateTexMat,
                         "osg::Object osgStupeflix::UpdateTexMat" )
{
    ADD_USER_SERIALIZER( Transforms );  // _initialBound
}

