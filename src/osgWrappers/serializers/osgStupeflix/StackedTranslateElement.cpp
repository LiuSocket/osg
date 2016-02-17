#include <osgAnimation/Sampler>
#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "Animation.h"


static bool checkSampler( const osgStupeflix::StupeflixStackedTranslateElement& stackedElement)
{
    return true;
}

static bool readSampler( osgDB::InputStream& is, osgStupeflix::StupeflixStackedTranslateElement& stackedElement)
{
    osgAnimation::TypedSampler<osg::Vec3>* sampler;

    sampler = readVec3Sampler(is);

    
    if (sampler == NULL) {
        throw "Sampler is NULL";
        return false;
    }

    stackedElement.setSampler(sampler);
    return true;
}


static bool writeSampler( osgDB::OutputStream& os, const osgStupeflix::StupeflixStackedTranslateElement& stackedElement )
{
    const osgAnimation::TypedSampler<osg::Vec3>* sampler = stackedElement.getSampler();
    return writeVec3Sampler(os, sampler);
}

REGISTER_OBJECT_WRAPPER( osgStupeflix_StupeflixStackedTranslateElement,
                         new osgStupeflix::StupeflixStackedTranslateElement,
                         osgStupeflix::StupeflixStackedTranslateElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedTranslateElement osgStupeflix::StupeflixStackedTranslateElement" )
{

    ADD_USER_SERIALIZER( Sampler );  // _channels
}
