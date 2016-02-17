#include "UniformAnimationCallback.h"

UNIFORM_ANIMATION_CALLBACK_WRAPPER(Double, double)

REGISTER_OBJECT_WRAPPER( osgStupeflix_UniformAnimationCallbackDouble,
                         new osgStupeflix::UniformAnimationCallback<double>,
                         osgStupeflix::UniformAnimationCallback<double>,
                         "osg::Object osgStupeflix::UniformAnimationCallback<double>" )
{
    
    ADD_USER_SERIALIZER( Sampler );  // _channels

}
