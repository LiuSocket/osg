#include <osgAnimation/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include <osgStupeflix/VariableSpeedCubicBezier>
#include <osgAnimation/CubicBezier>

// reading channel helpers

#include<osg/io_utils>
template <typename ContainerType, typename ValueType>
static void readContainer( osgDB::InputStream& is, ContainerType* container )
{
    typedef typename ContainerType::KeyType KeyType;
    bool hasContainer = false;

    is >> is.PROPERTY("KeyFrameContainer") >> hasContainer;
    if ( hasContainer )
    {
        unsigned int size = 0;
        size = is.readSize(); is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0f;
            ValueType value;
            is >> time >> value;
            container->push_back( KeyType(time, value) );
        }
        is >> is.END_BRACKET;
    }
}

template <typename ContainerType, typename ValueType, typename InternalValueType>
static void readContainer2( osgDB::InputStream& is, ContainerType* container )
{
    typedef typename ContainerType::KeyType KeyType;
    bool hasContainer = false;
    is >> is.PROPERTY("KeyFrameContainer") >> hasContainer;
    if ( hasContainer )
    {
        unsigned int size = 0;
        size = is.readSize(); is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0f;
            InternalValueType pos, ptIn, ptOut;
            is >> time >> pos >> ptIn >> ptOut; 
            container->push_back( KeyType(time, ValueType(pos, ptIn, ptOut)) );
        }
        is >> is.END_BRACKET;
    }
}

template <typename ContainerType, typename ValueType, typename InternalValueType>
static void readContainer3( osgDB::InputStream& is, ContainerType* container )
{
    typedef typename ContainerType::KeyType KeyType;
    bool hasContainer = false;
    is >> is.PROPERTY("KeyFrameContainer") >> hasContainer;
    if ( hasContainer )
    {
        unsigned int size = 0;
        size = is.readSize(); is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            double time = 0.0f;
            InternalValueType pos, ptIn, ptOut;
            osg::Vec2f influence0;
            osg::Vec2f influence1;
            is >> time >> pos >> ptIn >> ptOut >> influence0 >> influence1; 
            container->push_back( KeyType(time, ValueType(pos, ptIn, ptOut, influence0, influence1)) );
        }
        is >> is.END_BRACKET;
    }
}


#define READ_SAMPLER_FUNC( NAME, SAMPLER, CONTAINER, VALUE ) \
    if ( type==#NAME ) { \
        sampler = new SAMPLER; \
        readContainer<CONTAINER, VALUE>( is, ((SAMPLER*)sampler)->getOrCreateKeyframeContainer() ); \
        is >> is.END_BRACKET; \
    }

#define READ_SAMPLER_FUNC2( NAME, SAMPLER, CONTAINER, VALUE, INVALUE ) \
    if ( type==#NAME ) { \
        sampler = new SAMPLER; \
        readContainer2<CONTAINER, VALUE, INVALUE>( is, ((SAMPLER*) sampler)->getOrCreateKeyframeContainer() ); \
        is >> is.END_BRACKET; \
    }

#define READ_SAMPLER_FUNC3( NAME, SAMPLER, CONTAINER, VALUE, INVALUE ) \
    if ( type==#NAME ) { \
        sampler = new SAMPLER; \
        readContainer3<CONTAINER, VALUE, INVALUE>( is, ((SAMPLER*) sampler)->getOrCreateKeyframeContainer() ); \
        is >> is.END_BRACKET; \
    }


// writing channel helpers
template <typename ContainerType>
static void writeContainer( osgDB::OutputStream& os, ContainerType* container )
{
    os << os.PROPERTY("KeyFrameContainer") << (container!=NULL);
    if ( container!=NULL )
    {
        os.writeSize(container->size()); os << os.BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<container->size(); ++i )
        {
            os << (*container)[i].getTime() << (*container)[i].getValue() << std::endl;
        }
        os << os.END_BRACKET;
    }
    os << std::endl;
}

#define WRITE_SAMPLER_FUNC(object, NAME, SAMPLER, CONTAINER ) \
    const SAMPLER* sp_##NAME = dynamic_cast<const SAMPLER*>(object);  \
    if ( sp_##NAME ) { \
        os << os.PROPERTY("Type") << std::string(#NAME) << os.BEGIN_BRACKET << std::endl; \
        writeContainer<const CONTAINER>( os, sp_##NAME->getKeyframeContainerTyped() ); \
        os << os.END_BRACKET << std::endl; \
        found = true; \
    }

#define SIMPLE_WRITE_SAMPLER(PARAM,SHORTNAME, FULLNAME)                 \
bool write##SHORTNAME##PARAM( osgDB::OutputStream& os, const osgAnimation::TypedSampler<FULLNAME>* sampler)  \
{ \
    bool found = false;\
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##StepSampler, osgAnimation::SHORTNAME##StepSampler, osgAnimation::SHORTNAME##KeyframeContainer);    \
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##KeyframeContainer);\
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##CubicBezierSampler, osgAnimation::SHORTNAME##CubicBezierSampler, osgAnimation::SHORTNAME##CubicBezierKeyframeContainer );\
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##VariableSpeedCubicBezierSampler, osgAnimation::SHORTNAME##VariableSpeedCubicBezierSampler, osgAnimation::SHORTNAME##VariableSpeedCubicBezierKeyframeContainer );\
\
    if (! found) \
    {\
        return false;\
    }\
    return true;\
}

#define SIMPLE_READ_SAMPLER(PARAM,SHORTNAME, FULLNAME) \
osgAnimation::TypedSampler<FULLNAME>* read##SHORTNAME##PARAM( osgDB::InputStream& is) \
{\
    osgAnimation::TypedSampler<FULLNAME>* sampler = NULL;\
    std::string type;\
    is >> is.PROPERTY("Type") >> type >> is.BEGIN_BRACKET;\
        \
    READ_SAMPLER_FUNC(SHORTNAME##StepSampler, osgAnimation::SHORTNAME##StepSampler, osgAnimation::SHORTNAME##KeyframeContainer, FULLNAME);\
    READ_SAMPLER_FUNC(SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##KeyframeContainer, FULLNAME);\
    READ_SAMPLER_FUNC2(SHORTNAME##CubicBezierSampler, osgAnimation::SHORTNAME##CubicBezierSampler, osgAnimation::SHORTNAME##CubicBezierKeyframeContainer, osgAnimation::SHORTNAME##CubicBezier, FULLNAME);\
    READ_SAMPLER_FUNC3(SHORTNAME##VariableSpeedCubicBezierSampler, osgAnimation::SHORTNAME##VariableSpeedCubicBezierSampler, osgAnimation::SHORTNAME##VariableSpeedCubicBezierKeyframeContainer,\
     osgAnimation::SHORTNAME##VariableSpeedCubicBezier, FULLNAME);   \
\
    return sampler;\
}\



#define SIMPLE_WRITE_SAMPLER_DEFINITION(PARAM, SHORTNAME, FULLNAME)      \
bool write##SHORTNAME##PARAM( osgDB::OutputStream& os, const osgAnimation::TypedSampler<FULLNAME>* sampler);


#define SIMPLE_READ_SAMPLER_DEFINITION(PARAM,SHORTNAME, FULLNAME)       \
osgAnimation::TypedSampler<FULLNAME>* read##SHORTNAME##PARAM( osgDB::InputStream& is);


#define SIMPLE_READWRITE_SAMPLER(PARAM,SHORTNAME, FULLNAME)    \
  SIMPLE_WRITE_SAMPLER(PARAM,SHORTNAME,FULLNAME) \
  SIMPLE_READ_SAMPLER(PARAM,SHORTNAME,FULLNAME)

#define SIMPLE_READWRITE_SAMPLER_DEFINITION(PARAM,SHORTNAME,FULLNAME)   \
  SIMPLE_WRITE_SAMPLER_DEFINITION(PARAM,SHORTNAME,FULLNAME)             \
  SIMPLE_READ_SAMPLER_DEFINITION(PARAM,SHORTNAME,FULLNAME)



SIMPLE_READWRITE_SAMPLER_DEFINITION(Sampler,Float, float)
SIMPLE_READWRITE_SAMPLER_DEFINITION(Sampler,Double, double)
SIMPLE_READWRITE_SAMPLER_DEFINITION(Sampler,Vec2, osg::Vec2)
SIMPLE_READWRITE_SAMPLER_DEFINITION(Sampler,Vec3, osg::Vec3)
SIMPLE_READWRITE_SAMPLER_DEFINITION(Sampler,Vec4, osg::Vec4)


///////////////////////////////////////////////// NO CUBIC VERSION FOR QUATERNIONS ///////////////////////////////////////////


#define SIMPLE_WRITE_SAMPLER_NO_CUBIC(PARAM,SHORTNAME, FULLNAME) \
bool write##SHORTNAME##PARAM( osgDB::OutputStream& os, const osgAnimation::TypedSampler<FULLNAME>* sampler)  \
{ \
    bool found = false;\
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##StepSampler, osgAnimation::SHORTNAME##StepSampler, osgAnimation::SHORTNAME##KeyframeContainer);    \
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##KeyframeContainer);\
\
    if (! found) \
    {\
        return false;\
    }\
    return true;\
}

#define SIMPLE_READ_SAMPLER_NO_CUBIC(PARAM,SHORTNAME, FULLNAME)         \
osgAnimation::TypedSampler<FULLNAME>* read##SHORTNAME##PARAM( osgDB::InputStream& is) \
{\
    osgAnimation::TypedSampler<FULLNAME>* sampler = NULL;\
    std::string type;\
    is >> is.PROPERTY("Type") >> type >> is.BEGIN_BRACKET;\
        \
    READ_SAMPLER_FUNC(SHORTNAME##StepSampler, osgAnimation::SHORTNAME##StepSampler, osgAnimation::SHORTNAME##KeyframeContainer, FULLNAME);\
    READ_SAMPLER_FUNC(SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##KeyframeContainer, FULLNAME);\
\
    return sampler;\
}\



#define SIMPLE_WRITE_SAMPLER_NO_CUBIC_DEFINITION(PARAM,SHORTNAME, FULLNAME)                    \
bool write##SHORTNAME##PARAM( osgDB::OutputStream& os, const osgAnimation::TypedSampler<FULLNAME>* sampler);


#define SIMPLE_READ_SAMPLER_NO_CUBIC_DEFINITION(PARAM,SHORTNAME, FULLNAME)                           \
osgAnimation::TypedSampler<FULLNAME>* read##SHORTNAME##PARAM( osgDB::InputStream& is);


#define SIMPLE_READWRITE_SAMPLER_NO_CUBIC(PARAM,SHORTNAME, FULLNAME)         \
  SIMPLE_WRITE_SAMPLER_NO_CUBIC(PARAM,SHORTNAME,FULLNAME)\
  SIMPLE_READ_SAMPLER_NO_CUBIC(PARAM,SHORTNAME,FULLNAME)

#define SIMPLE_READWRITE_SAMPLER_NO_CUBIC_DEFINITION(PARAM,SHORTNAME,FULLNAME)     \
  SIMPLE_WRITE_SAMPLER_NO_CUBIC_DEFINITION(PARAM,SHORTNAME,FULLNAME)\
  SIMPLE_READ_SAMPLER_NO_CUBIC_DEFINITION(PARAM,SHORTNAME,FULLNAME)



SIMPLE_READWRITE_SAMPLER_NO_CUBIC_DEFINITION(Sampler,Quat, osg::Quat)



///////////////////////////////////////////////// ONLY LINEAR VERSION FOR QUATERNIONS ///////////////////////////////////////////


#define SIMPLE_WRITE_SAMPLER_ONLY_LINEAR(PARAM,SHORTNAME, FULLNAME) \
bool write##SHORTNAME##PARAM( osgDB::OutputStream& os, const osgAnimation::TypedSampler<FULLNAME>* sampler)  \
{ \
    bool found = false;\
    WRITE_SAMPLER_FUNC(sampler, SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##KeyframeContainer);\
\
    if (! found) \
    {\
        return false;\
    }\
    return true;\
}

#define SIMPLE_READ_SAMPLER_ONLY_LINEAR(PARAM,SHORTNAME, FULLNAME)  \
osgAnimation::TypedSampler<FULLNAME>* read##SHORTNAME##PARAM( osgDB::InputStream& is) \
{\
    osgAnimation::TypedSampler<FULLNAME>* sampler = NULL;\
    std::string type;\
    is >> is.PROPERTY("Type") >> type >> is.BEGIN_BRACKET;\
        \
    READ_SAMPLER_FUNC(SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##LinearSampler, osgAnimation::SHORTNAME##KeyframeContainer, FULLNAME);\
\
    return sampler;\
}\


#define SIMPLE_WRITE_SAMPLER_ONLY_LINEAR_DEFINITION(PARAM,SHORTNAME, FULLNAME)                    \
bool write##SHORTNAME##PARAM( osgDB::OutputStream& os, const osgAnimation::TypedSampler<FULLNAME>* sampler);


#define SIMPLE_READ_SAMPLER_ONLY_LINEAR_DEFINITION(PARAM,SHORTNAME, FULLNAME)                           \
osgAnimation::TypedSampler<FULLNAME>* read##SHORTNAME##PARAM( osgDB::InputStream& is);


#define SIMPLE_READWRITE_SAMPLER_ONLY_LINEAR(PARAM,SHORTNAME, FULLNAME)         \
  SIMPLE_WRITE_SAMPLER_ONLY_LINEAR(PARAM,SHORTNAME,FULLNAME)\
  SIMPLE_READ_SAMPLER_ONLY_LINEAR(PARAM,SHORTNAME,FULLNAME)

#define SIMPLE_READWRITE_SAMPLER_ONLY_LINEAR_DEFINITION(PARAM,SHORTNAME,FULLNAME)     \
  SIMPLE_WRITE_SAMPLER_ONLY_LINEAR_DEFINITION(PARAM,SHORTNAME,FULLNAME)\
  SIMPLE_READ_SAMPLER_ONLY_LINEAR_DEFINITION(PARAM,SHORTNAME,FULLNAME)



SIMPLE_READWRITE_SAMPLER_ONLY_LINEAR_DEFINITION(Sampler,Matrix, osg::Matrixf)
