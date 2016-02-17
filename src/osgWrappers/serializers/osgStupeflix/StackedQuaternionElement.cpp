#include <osgAnimation/Sampler>
#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "Animation.h"


static bool checkSampler( const osgStupeflix::StupeflixStackedQuaternionElement& stackedElement)
{
    return true;
}

static bool readSampler( osgDB::InputStream& is, osgStupeflix::StupeflixStackedQuaternionElement& stackedElement)
{
    osgAnimation::TypedSampler<osg::Quat>* sampler;

    sampler = readQuatSampler(is);

    
    if (sampler == NULL) {
        return false;
    }

    stackedElement.setSampler(sampler);
    return true;
}


static bool writeSampler( osgDB::OutputStream& os, const osgStupeflix::StupeflixStackedQuaternionElement& stackedElement )
{
    const osgAnimation::TypedSampler<osg::Quat>* sampler = stackedElement.getSampler();
    return writeQuatSampler(os, sampler);
}

REGISTER_OBJECT_WRAPPER( osgStupeflix_StupeflixStackedQuaternionElement,
                         new osgStupeflix::StupeflixStackedQuaternionElement,
                         osgStupeflix::StupeflixStackedQuaternionElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedQuaternionElement osgStupeflix::StupeflixStackedQuaternionElement" )
{

    ADD_USER_SERIALIZER( Sampler );  // _channels
}
