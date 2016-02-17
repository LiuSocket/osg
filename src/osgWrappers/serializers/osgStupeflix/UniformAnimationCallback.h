#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "Animation.h"
#include "osgStupeflix/Sampler"


#define UNIFORM_ANIMATION_CALLBACK_WRAPPER(NAME, VALUETYPE) \
template<class T> bool checkSampler( const osgStupeflix::UniformAnimationCallback<T>& uniformAnimationCallBack)\
{\
return true;\
}\
\
bool readSampler( osgDB::InputStream& is, osgStupeflix::UniformAnimationCallback<VALUETYPE>& uniformAnimationCallBack)\
{\
osgAnimation::TypedSampler<VALUETYPE>* sampler;\
\
sampler = read##NAME##Sampler(is);\
\
if (sampler == NULL) {\
return false;\
}\
\
uniformAnimationCallBack.setSampler(sampler);\
return true;\
}\
\
template<class T> bool writeSampler( osgDB::OutputStream& os, const osgStupeflix::UniformAnimationCallback<T>& uniformAnimationCallBack )\
{\
const osgAnimation::TypedSampler<VALUETYPE>* sampler = uniformAnimationCallBack.getSampler();\
return write##NAME##Sampler(os, sampler);\
}
