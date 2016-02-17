#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgStupeflix/Animation>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>
#include "UpdateMatrixTransform.h"

UPDATE_MATRIX_TRANSFORM_WRAPPER(Position, Vec3, osg::Vec3)
UPDATE_MATRIX_TRANSFORM_WRAPPER(Scale, Vec3, osg::Vec3)
UPDATE_MATRIX_TRANSFORM_WRAPPER(AxisRotation, Vec4, osg::Vec4)
UPDATE_MATRIX_TRANSFORM_WRAPPER(Matrix, Matrix, osg::Matrixf)
UPDATE_MATRIX_TRANSFORM_WRAPPER(QuaternionRotation, Quat, osg::Quat)

REGISTER_OBJECT_WRAPPER( osgStupeflix_UpdateMatrixTransform,
                         new osgStupeflix::UpdateMatrixTransform,
                         osgStupeflix::UpdateMatrixTransform,
                         "osg::Object osg::NodeCallback osgStupeflix::UpdateMatrixTransform")
{
    ADD_USER_SERIALIZER(Position);
    ADD_USER_SERIALIZER(Scale);
    ADD_USER_SERIALIZER(AxisRotation);
    ADD_USER_SERIALIZER(Matrix);
    ADD_USER_SERIALIZER(QuaternionRotation);
}

