#include "UniformAnimationCallback.h"

UNIFORM_ANIMATION_CALLBACK_WRAPPER(Matrix, osg::Matrixf)

REGISTER_OBJECT_WRAPPER( osgStupeflix_UniformAnimationCallbackMatrixf,
                         new osgStupeflix::UniformAnimationCallback<osg::Matrixf>,
                         osgStupeflix::UniformAnimationCallback<osg::Matrixf>,
                         "osg::Object osgStupeflix::UniformAnimationCallback<osg::Matrixf>" )
{
    
    ADD_USER_SERIALIZER( Sampler );  // _channels

}
