#include "UniformAnimationCallback.h"

UNIFORM_ANIMATION_CALLBACK_WRAPPER(Float, float)

REGISTER_OBJECT_WRAPPER( osgStupeflix_UniformAnimationCallbackFloat,
                         new osgStupeflix::UniformAnimationCallback<float>,
                         osgStupeflix::UniformAnimationCallback<float>,
                         "osg::Object osgStupeflix::UniformAnimationCallback<float>" )
{
    
    ADD_USER_SERIALIZER( Sampler );  // _channels

}
