#include "UniformAnimationCallback.h"
UNIFORM_ANIMATION_CALLBACK_WRAPPER(Vec4, osg::Vec4)

REGISTER_OBJECT_WRAPPER( osgStupeflix_UniformAnimationCallbackVec4f,
                         new osgStupeflix::UniformAnimationCallback<osg::Vec4f>,
                         osgStupeflix::UniformAnimationCallback<osg::Vec4f>,
                         "osg::Object osgStupeflix::UniformAnimationCallback<osg::Vec4f>" )
{
    
    ADD_USER_SERIALIZER( Sampler );  // _channels
}
