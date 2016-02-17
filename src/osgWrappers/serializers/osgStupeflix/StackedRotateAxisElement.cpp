#include <osgAnimation/Sampler>
#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "Animation.h"


static bool checkSampler( const osgStupeflix::StupeflixStackedRotateAxisElement& stackedElement)
{
    return true;
}

static bool readSampler( osgDB::InputStream& is, osgStupeflix::StupeflixStackedRotateAxisElement& stackedElement)
{
    osgAnimation::TypedSampler<osg::Vec4>* sampler;

    sampler = readVec4Sampler(is);

    
    if (sampler == NULL) {
        throw "Sampler is NULL";
        return false;
    }

    stackedElement.setSampler(sampler);
    return true;
}


static bool writeSampler( osgDB::OutputStream& os, const osgStupeflix::StupeflixStackedRotateAxisElement& stackedElement )
{
    const osgAnimation::TypedSampler<osg::Vec4>* sampler = stackedElement.getSampler();
    return writeVec4Sampler(os, sampler);
}

REGISTER_OBJECT_WRAPPER( osgStupeflix_StupeflixStackedRotateAxisElement,
                         new osgStupeflix::StupeflixStackedRotateAxisElement,
                         osgStupeflix::StupeflixStackedRotateAxisElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedRotateAxisElement osgStupeflix::StupeflixStackedRotateAxisElement" )
{

    ADD_USER_SERIALIZER( Sampler );  // _channels
}
