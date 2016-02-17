#include "UniformAnimationCallback.h"
UNIFORM_ANIMATION_CALLBACK_WRAPPER(Vec3, osg::Vec3)



REGISTER_OBJECT_WRAPPER( osgStupeflix_UniformAnimationCallbackVec3f,
                         new osgStupeflix::UniformAnimationCallback<osg::Vec3f>,
                         osgStupeflix::UniformAnimationCallback<osg::Vec3f>,
                         "osg::Object osg::Uniform::Callback osgStupeflix::UniformAnimationCallback<osg::Vec3f>" )

{
    
    ADD_USER_SERIALIZER( Sampler );  // _channels

}
