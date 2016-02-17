#include "UniformAnimationCallback.h"

UNIFORM_ANIMATION_CALLBACK_WRAPPER(Vec2, osg::Vec2)

REGISTER_OBJECT_WRAPPER( osgStupeflix_UniformAnimationCallbackVec2f,
                         new osgStupeflix::UniformAnimationCallback<osg::Vec2f>,
                         osgStupeflix::UniformAnimationCallback<osg::Vec2f>,
                         "osg::Object osgStupeflix::UniformAnimationCallback<osg::Vec2f>" )
{
    
    ADD_USER_SERIALIZER( Sampler );  // _channels

}
