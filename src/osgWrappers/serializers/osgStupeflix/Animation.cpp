#include "Animation.h"
#include "osgStupeflix/Animation"
#include "osgStupeflix/VariableSpeedCubicBezier"
#include "osgStupeflix/Sampler"

using namespace osgStupeflix;




SIMPLE_READWRITE_SAMPLER(Sampler,Float, float)
SIMPLE_READWRITE_SAMPLER(Sampler,Double, double)
SIMPLE_READWRITE_SAMPLER(Sampler,Vec2, osg::Vec2)
SIMPLE_READWRITE_SAMPLER(Sampler,Vec3, osg::Vec3)
SIMPLE_READWRITE_SAMPLER(Sampler,Vec4, osg::Vec4)


#define QuatLinearSampler QuatSphericalLinearSampler

SIMPLE_READWRITE_SAMPLER_NO_CUBIC(Sampler,Quat, osg::Quat)
SIMPLE_READWRITE_SAMPLER_ONLY_LINEAR(Sampler,Matrix, osg::Matrixf)
