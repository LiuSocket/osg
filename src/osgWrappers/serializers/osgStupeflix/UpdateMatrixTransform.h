#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "Animation.h"


#define UPDATE_MATRIX_TRANSFORM_WRAPPER(PARAM, NAME, VALUETYPE)\
bool check##PARAM(const osgStupeflix::UpdateMatrixTransform& updateMatrixTransform)\
{\
    return true;\
    const osgAnimation::TypedSampler<VALUETYPE>* sampler = updateMatrixTransform.get##PARAM();\
    return sampler != NULL;\
}\
bool read##PARAM( osgDB::InputStream& is, osgStupeflix::UpdateMatrixTransform& updateMatrixTransform)\
{\
    osgAnimation::TypedSampler<VALUETYPE>* sampler;\
\
    bool test;\
    is >> test;\
    if (test) {\
        is >> is.BEGIN_BRACKET;\
        sampler = read##NAME##Sampler(is);      \
        is >> is.END_BRACKET;\
    } else {\
        sampler = NULL;\
    }\
\
    updateMatrixTransform.set##PARAM(sampler);\
    return true;\
}\
\
bool write##PARAM( osgDB::OutputStream& os, const osgStupeflix::UpdateMatrixTransform& updateMatrixTransform )\
{\
   const osgAnimation::TypedSampler<VALUETYPE>* sampler = updateMatrixTransform.get##PARAM();\
   if (sampler == NULL) {\
        os << false << std::endl;                 \
        return true; \
   }\
   os << true << os.BEGIN_BRACKET<<std::endl;                            \
   bool ret =  write##NAME##Sampler(os, sampler);\
   os << os.END_BRACKET << std::endl; \
   return ret;\
}
