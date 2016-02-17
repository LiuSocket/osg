#include <osgAnimation/Sampler>
#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "Animation.h"


static bool checkSampler( const osgStupeflix::StupeflixStackedScaleElement& stackedElement)
{
    return true;
}

static bool readSampler( osgDB::InputStream& is, osgStupeflix::StupeflixStackedScaleElement& stackedElement)
{
    osgAnimation::TypedSampler<osg::Vec3>* sampler;

    sampler = readVec3Sampler(is);

    
    if (sampler == NULL) {
        return false;
    }

    stackedElement.setSampler(sampler);
    return true;
}


static bool writeSampler( osgDB::OutputStream& os, const osgStupeflix::StupeflixStackedScaleElement& stackedElement )
{
    const osgAnimation::TypedSampler<osg::Vec3>* sampler = stackedElement.getSampler();
    return writeVec3Sampler(os, sampler);
}

REGISTER_OBJECT_WRAPPER( osgStupeflix_StupeflixStackedScaleElement,
                         new osgStupeflix::StupeflixStackedScaleElement,
                         osgStupeflix::StupeflixStackedScaleElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedScaleElement osgStupeflix::StupeflixStackedScaleElement" )
{

    ADD_USER_SERIALIZER( Sampler );  // _channels
}
