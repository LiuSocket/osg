/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgStupeflix/Animation>
#include <osgStupeflix/Interpolator>
#include <osgStupeflix/VariableSpeedCubicBezier>
#include <osgStupeflix/TexMat>
#include <osgStupeflix/Sampler>
#include <osg/NodeVisitor>
#include <osg/TexMat>
#include <osg/StateAttributeCallback>
#include <osg/io_utils>
#include <algorithm>

using namespace osgStupeflix;


void osgStupeflix::setAnimationUniformFloat(osg::Uniform* uniform, osgAnimation::TemplateKeyframeContainer<float>* keys)
{
    UniformAnimationCallback<float>* cb = new UniformAnimationCallback<float>;
    osgAnimation::FloatLinearSampler* sampler = new osgAnimation::FloatLinearSampler();
    sampler->setKeyframeContainer(keys);
    cb->setSampler(sampler);
    uniform->setUpdateCallback(cb);
}

void osgStupeflix::setAnimationUniformDouble(osg::Uniform* uniform, osgAnimation::TemplateKeyframeContainer<double>* keys)
{
    UniformAnimationCallback<double>* cb = new UniformAnimationCallback<double>;
    osgAnimation::DoubleLinearSampler* sampler = new osgAnimation::DoubleLinearSampler();
    sampler->setKeyframeContainer(keys);
    cb->setSampler(sampler);
    uniform->setUpdateCallback(cb);
}


#define SET_ANIMATION_UNIFORM_VEC(DIM) \
void osgStupeflix::setAnimationUniformVec##DIM(osg::Uniform* uniform, osgAnimation::TemplateKeyframeContainer<osg::Vec##DIM>* keys)\
{\
    UniformAnimationCallback<osg::Vec##DIM>* cb = new UniformAnimationCallback<osg::Vec##DIM>;\
    osgAnimation::Vec##DIM##LinearSampler* sampler = new osgAnimation::Vec##DIM##LinearSampler();\
    sampler->setKeyframeContainer(keys);\
    cb->setSampler(sampler); \
    uniform->setUpdateCallback(cb);\
}\
\
void osgStupeflix::setAnimationUniformVariableSpeedCubicBezierVec##DIM(osg::Uniform* uniform, osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec##DIM##VariableSpeedCubicBezier>* keys)\
{\
    UniformAnimationCallback<osg::Vec##DIM>* cb = new UniformAnimationCallback<osg::Vec##DIM>;\
    osgAnimation::Vec##DIM##VariableSpeedCubicBezierSampler* sampler = new osgAnimation::Vec##DIM##VariableSpeedCubicBezierSampler();\
    sampler->setKeyframeContainer(keys);\
    cb->setSampler(sampler); \
    uniform->setUpdateCallback(cb);\
}


SET_ANIMATION_UNIFORM_VEC(2)
SET_ANIMATION_UNIFORM_VEC(3)
SET_ANIMATION_UNIFORM_VEC(4)

void osgStupeflix::setAnimationUniformMatrix(osg::Uniform* uniform, osgAnimation::TemplateKeyframeContainer<osg::Matrixf>* keys)
{
    UniformAnimationCallback<osg::Matrixf>* cb = new UniformAnimationCallback<osg::Matrixf>;
    osgAnimation::MatrixLinearSampler* sampler = new osgAnimation::MatrixLinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setSampler(sampler);
    uniform->setUpdateCallback(cb);
}


#define UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(type) \
  {\
    osgStupeflix::UniformAnimationCallback<type> * cb_check = dynamic_cast<osgStupeflix::UniformAnimationCallback<type> *>(cb); \
    if (cb_check != NULL)  { \
        cb_check->setStartAndTimeStretch(startTime, timeStretch);\
    }\
  }


void osgStupeflix::uniformAnimationCallbackSetTimeStretch(osg::Uniform* uniform, float startTime, float timeStretch) {
    osg::Uniform::Callback* cb = uniform->getUpdateCallback();

    UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(float);
    UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(double);
    UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(osg::Vec2);
    UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(osg::Vec3);
    UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(osg::Vec4);
    UNIFORM_ANIMATION_CALLBACK_TIME_STRETCH_TRY_TYPE(osg::Matrixf);

}

void osgStupeflix::stateSetTimeStretch(osg::StateSet* stateSet, float startTime, float timeStretch) {
    osg::StateSet::UniformList& uniformList = stateSet->getUniformList();

    for(osg::StateSet::UniformList::iterator it = uniformList.begin(); it != uniformList.end(); ++it )
    {
        osg::StateSet::RefUniformPair& value = it->second;
        osg::Uniform* uniform = value.first;
        uniformAnimationCallbackSetTimeStretch(uniform, startTime, timeStretch);
    }

}

osgStupeflix::UpdateMatrixTransform* osgStupeflix::getMatrixUpdateCallback(osg::MatrixTransform* transform)
{
    if (!transform)
        return 0;

    osg::NodeCallback* cb = transform->getUpdateCallback();
    while (cb) {
        UpdateMatrixTransform*acb = dynamic_cast<UpdateMatrixTransform*>(cb);
        if (acb)
            return acb;
        cb = cb->getNestedCallback();
    }
    return 0;
}


void osgStupeflix::nodeTimeStretch(osg::Node* node, float startTime, float timeStretch) {
    osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node);
    if (transform) {
        osgStupeflix::UpdateMatrixTransform* updateCB = osgStupeflix::getMatrixUpdateCallback(transform);
        if (updateCB) {
            updateCB->setStartAndTimeStretch(startTime, timeStretch);
        }
    }
}



void osgStupeflix::setAnimationPositionVec3(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setPosition(sampler);
}

void osgStupeflix::setAnimationVariableSpeedCubicBezierPositionVec3(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec3VariableSpeedCubicBezier>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::Vec3VariableSpeedCubicBezierSampler* sampler = new osgAnimation::Vec3VariableSpeedCubicBezierSampler;
    sampler->setKeyframeContainer(keys);
    cb->setPosition(sampler);
}



void osgStupeflix::setAnimationVariableSpeedCubicBezierScaleVec3(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec3VariableSpeedCubicBezier>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::Vec3VariableSpeedCubicBezierSampler* sampler = new osgAnimation::Vec3VariableSpeedCubicBezierSampler;
    sampler->setKeyframeContainer(keys);
    cb->setScale(sampler);
}



void osgStupeflix::setAnimationAxisRotation(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec4f>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::Vec4LinearSampler* sampler = new osgAnimation::Vec4LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setAxisRotation(sampler);
}


void osgStupeflix::setAnimationVariableSpeedCubicBezierAxisRotation(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec4VariableSpeedCubicBezier>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::Vec4VariableSpeedCubicBezierSampler* sampler = new osgAnimation::Vec4VariableSpeedCubicBezierSampler;
    sampler->setKeyframeContainer(keys);
    cb->setAxisRotation(sampler);
}


void osgStupeflix::setAnimationScaleVec3(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setScale(sampler);

}


void osgStupeflix::setAnimationQuaternionRotation(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osg::Quat>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::QuatSphericalLinearSampler* sampler = new osgAnimation::QuatSphericalLinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setQuaternionRotation(sampler);
}

void osgStupeflix::setAnimationMatrix(osg::MatrixTransform* matrix, osgAnimation::TemplateKeyframeContainer<osg::Matrixf>* keys)
{
    UpdateMatrixTransform* cb = getMatrixUpdateCallback(matrix);
    if (!cb) {
        cb = new UpdateMatrixTransform;
        matrix->addUpdateCallback(cb);
    }
    osgAnimation::MatrixLinearSampler* sampler = new osgAnimation::MatrixLinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setMatrix(sampler);
}


typedef std::vector<osg::ref_ptr<osgAnimation::TypedSampler<osg::Vec3> > > Vec3SamplerVector;

struct StupeflixStackedPointOfInterestElement : public osgAnimation::StackedMatrixElement
{
    StupeflixStackedPointOfInterestElement() {
        _up = osg::Vec3(0,1,0);
    }

    void setPointOfInterest(osgAnimation::TypedSampler<osg::Vec3> * sampler){
        _sampler = sampler;
    }

    void addPosition(osgAnimation::TypedSampler<osg::Vec3> * sampler) {
        _position.push_back(sampler);
    }

    void setUpVector(osg::Vec3 up) {
        _up = up;
    }

    void update(float t) {
        osg::Vec3 interest;
        _sampler->getValueAt(t, interest);
        osg::Vec3 translate;


        for (Vec3SamplerVector::iterator it = _position.begin(); it != _position.end(); ++it) {
            osg::Vec3 translate_part;
            osg::ref_ptr<osgAnimation::TypedSampler<osg::Vec3> > positionSampler = *it;
            positionSampler->getValueAt(t, translate_part);
            translate += translate_part;
        }

        _matrix.makeLookAt(osg::Vec3(0.0, 0.0, 0.0), interest - translate, _up);
        //        osg::notify(osg::INFO) << "translate " << translate << " POI " << _matrix << std::endl;
    }

    osg::ref_ptr<osgAnimation::TypedSampler<osg::Vec3> > _sampler;
    Vec3SamplerVector _position;
    osg::Vec3 _up;
};



class UpdateCamera : public osg::NodeCallback
{
public:

    UpdateCamera() {
        _aspectRatio = 16.0/9.0;
        _near = 0.01;
        _far = 100.0;
        _up = osg::Vec3(0,1,0);
        _pointOfInterest = new StupeflixStackedPointOfInterestElement();
        _reversed = false;
    }

    // Callback method called by the NodeVisitor when visiting a node.
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
        // First time we have to reverse the stack of transforms that were added, as we are building the inverse transfomrm
        // (each transform is also reversed : -translate, -rotate, 1/scale etc 
        if (! _reversed) {
            std::reverse(_transforms.begin(),_transforms.end());
            _reversed = true;
        }

        double t = nv->getFrameStamp()->getSimulationTime();
        osg::Camera* camera = static_cast<osg::Camera*>(node);

        if (_viewAngle.valid()) {
            float viewAngle = 0;
            _viewAngle->getValueAt(t, viewAngle);
            camera->setProjectionMatrixAsPerspective(viewAngle,
                                                     _aspectRatio,
                                                     _near,
                                                     _far);
        }

        _transforms.update(t);
        const osg::Matrix& result = _transforms.getMatrix();


        //        osg::notify(osg::INFO) << "osgStupeflix: " << camera->getName() << " time " << t << " " << result << std::endl;
        camera->setViewMatrix(result);

        traverse(camera,nv);
    }

    void addTransform(osgAnimation::StackedTransformElement* element) {
        _transforms.push_back(element);
    }

    void addPosition(osgAnimation::TypedSampler<osg::Vec3> * sampler, bool addToPOI)
    {
        addTransform(new StupeflixStackedTranslateElement(sampler, true));

        if (addToPOI) {
            _pointOfInterest->addPosition(sampler);
        }
    }

    void addScale(osgAnimation::TypedSampler<osg::Vec3> * sampler) {
        addTransform(new StupeflixStackedScaleElement(sampler, true));
    }

    void addQuaternionRotation(osgAnimation::QuatSphericalLinearSampler* sampler)
    {
        addTransform(new StupeflixStackedQuaternionElement(sampler, true));
    }

    void addAxisRotation(osgAnimation::TypedSampler<osg::Vec4> * sampler)
    {
        addTransform(new StupeflixStackedRotateAxisElement(sampler, true));
    }

    void setPointOfInterest(osgAnimation::TypedSampler<osg::Vec3> * sampler)
    {
        _pointOfInterest.get()->setPointOfInterest(sampler);
        addTransform(_pointOfInterest.get());
        _pointOfInterest->setUpVector(_up);
    }

    void setNearFar(float near, float far)
    {
        _near = near;
        _far = far;
    }

    void setAspectRatio(float aspect)
    {
        _aspectRatio = aspect;
    }

    void setAngleView(osgAnimation::TypedSampler<float> * sampler)
    {
        _viewAngle = sampler;
    }

protected:
    osgAnimation::StackedTransform _transforms;
    osg::ref_ptr<osgAnimation::TypedSampler<float> > _viewAngle;
    Vec3SamplerVector _position;
    osg::ref_ptr<StupeflixStackedPointOfInterestElement> _pointOfInterest;
    osg::Vec3 _up;
    float _near, _far;
    float _aspectRatio;
    bool _reversed;
};



static UpdateCamera* getOrCreateCameraUpdateCallback(osg::Camera* node)
{
    UpdateCamera* acb = 0;
    if (!node) {
        osg::notify(osg::WARN) << "no camera given as parameter" << std::endl;
        return 0;
    }

    osg::NodeCallback* cb = node->getUpdateCallback();
    while (cb) {
        acb = dynamic_cast<UpdateCamera*>(cb);
        if (acb)
            return acb;
        cb = cb->getNestedCallback();
    }

    acb = new UpdateCamera;
    node->addUpdateCallback(acb);
    return acb;
}


void osgStupeflix::addCameraAnimationPositionVec3(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys, bool addToPOI)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->addPosition(sampler, addToPOI);
}

void osgStupeflix::addCameraAnimationVariableSpeedCubicBezierPositionVec3(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec3VariableSpeedCubicBezier>* keys, bool addToPOI)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);

    osgAnimation::Vec3VariableSpeedCubicBezierSampler* sampler = new osgAnimation::Vec3VariableSpeedCubicBezierSampler;
    sampler->setKeyframeContainer(keys);
    cb->addPosition(sampler, addToPOI);
}

void osgStupeflix::addCameraAnimationScaleVec3(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->addScale(sampler);
}


void osgStupeflix::addCameraAnimationQuaternionRotation(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osg::Quat>* keys)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::QuatSphericalLinearSampler* sampler = new osgAnimation::QuatSphericalLinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->addQuaternionRotation(sampler);
}

void osgStupeflix::addCameraAnimationAxisRotation(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec4f>* keys)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::Vec4LinearSampler* sampler = new osgAnimation::Vec4LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->addAxisRotation(sampler);
}

void osgStupeflix::addCameraAnimationVariableSpeedCubicBezierAxisRotation(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec4VariableSpeedCubicBezier>* keys)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::Vec4VariableSpeedCubicBezierSampler* sampler = new osgAnimation::Vec4VariableSpeedCubicBezierSampler;
    sampler->setKeyframeContainer(keys);
    cb->addAxisRotation(sampler);
}

void osgStupeflix::setCameraAnimationPointOfInterest(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setPointOfInterest(sampler);
}

void osgStupeflix::setCameraAnimationViewAngle(osg::Camera* matrix, osgAnimation::TemplateKeyframeContainer<float>* keys)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(matrix);
    osgAnimation::FloatLinearSampler* sampler = new osgAnimation::FloatLinearSampler;
    sampler->setKeyframeContainer(keys);
    cb->setAngleView(sampler);
}

void osgStupeflix::setCameraNearFar(osg::Camera* camera, float near, float far)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(camera);
    cb->setNearFar(near, far);
}

void osgStupeflix::setCameraAspectRatio(osg::Camera* camera, float aspect)
{
    UpdateCamera* cb = getOrCreateCameraUpdateCallback(camera);
    cb->setAspectRatio(aspect);
}


UpdateTexMat::UpdateTexMat():
  _startTime(0.0),
  _timeStretch(1.0)
 {
}

UpdateTexMat::UpdateTexMat(const UpdateTexMat& updateTexMat,const osg::CopyOp& copyop):
  _startTime(0.0),
  _timeStretch(1.0)
{
    _transforms  = updateTexMat._transforms;
}

// Callback method called by the NodeVisitor when visiting a node.
void UpdateTexMat::operator()(osg::StateAttribute* node, osg::NodeVisitor* nv) {

    double t = nv->getFrameStamp()->getSimulationTime();
    t = (t - _startTime) * _timeStretch;
    osgStupeflix::TexMat* texMat = static_cast<osgStupeflix::TexMat*>(node);

    if (texMat == NULL) {
        osg::notify(osg::INFO) << "UpdateTexMat with non osgStupeflix texMat" << std::endl;
        return;
    }
    _transforms.update(t);
    const osg::Matrix& result = _transforms.getMatrix();

    texMat->setMatrix(result);
    texMat->addToParentStateSet();
    //osg::notify(osg::INFO) << "UpdateTexMat " << result << std::endl;

}


void UpdateTexMat::addMatrix(osgAnimation::TemplateKeyframeContainer<osg::Matrixf>* keys) {
    osgAnimation::MatrixLinearSampler* sampler = new osgAnimation::MatrixLinearSampler;
    sampler->setKeyframeContainer(keys);
    _transforms.push_back(new StupeflixStackedMatrixElement(sampler));
}

void UpdateTexMat::addTranslate(osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys)
{
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    _transforms.push_back(new StupeflixStackedTranslateElement(sampler));
}

void UpdateTexMat::addScale(osgAnimation::TemplateKeyframeContainer<osg::Vec3f>* keys) {
    osgAnimation::Vec3LinearSampler* sampler = new osgAnimation::Vec3LinearSampler;
    sampler->setKeyframeContainer(keys);
    _transforms.push_back(new StupeflixStackedScaleElement(sampler));
}

void UpdateTexMat::addQuaternionRotation(osgAnimation::TemplateKeyframeContainer<osg::Quat>* keys)
{
    osgAnimation::QuatSphericalLinearSampler* sampler = new osgAnimation::QuatSphericalLinearSampler;
    sampler->setKeyframeContainer(keys);
    _transforms.push_back(new StupeflixStackedQuaternionElement(sampler));
}

void UpdateTexMat::addAxisRotation(osgAnimation::TemplateKeyframeContainer<osg::Vec4f>* keys)
{
    osgAnimation::Vec4LinearSampler* sampler = new osgAnimation::Vec4LinearSampler;
    sampler->setKeyframeContainer(keys);
    _transforms.push_back(new StupeflixStackedRotateAxisElement(sampler));
}


UpdateTexMat* osgStupeflix::getOrCreateTexMatUpdateCallback(osg::StateAttribute* stateAttribute)
{
    //    osg::notify(osg::INFO) << "UpdateTexMat getOrCreateTexMatUpdateCallback" << std::endl;
    UpdateTexMat* acb = 0;
    if (!stateAttribute) {
        osg::notify(osg::WARN) << "no stateAttribute given as parameter" << std::endl;
        return 0;
    }

    osg::StateAttributeCallback* cb = stateAttribute->getUpdateCallback();
    if (cb) {
        acb = dynamic_cast<UpdateTexMat*>(cb);
        if (acb) {
            return acb;
        } else {
            // We check if this is a default callback, that we can override
            DefaultUpdateTexMat* acb1 = dynamic_cast<DefaultUpdateTexMat*>(cb);

            if (acb1 == NULL) {
                // It's not a default callback, we don't take the risk to override it
                osg::notify(osg::WARN) << "existing callback with bad type" << std::endl;
                return 0;
            }
        }
    }

    acb = new UpdateTexMat;
    stateAttribute->setUpdateCallback(acb);

    //    osg::notify(osg::INFO) << "UpdateTexMat getOrCreateTexMatUpdateCallback added callback" << std::endl;

    return acb;
}

// Debugging code


osgAnimation::TemplateSampler<osgAnimation::Vec3VariableSpeedCubicBezierInterpolator>* osgStupeflix::vec3VariableSpeedCubicBezierSamplerCreate(osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec3VariableSpeedCubicBezier>* keys)
{
    osgAnimation::TemplateSampler<osgAnimation::Vec3VariableSpeedCubicBezierInterpolator>* sampler = new osgAnimation::TemplateSampler<osgAnimation::Vec3VariableSpeedCubicBezierInterpolator>;
    sampler->setKeyframeContainer(keys);
    return sampler;
}

osg::Vec3 osgStupeflix::vec3VariableSpeedCubicBezierSamplerGetValue(osgAnimation::TemplateSampler<osgAnimation::Vec3VariableSpeedCubicBezierInterpolator>* sampler, double time) {
    osg::Vec3 v;
    sampler->getValueAt(time, v);
    return v;
}



void StupeflixStackedTranslateElement::update(float t) {
    _sampler->getValueAt(t, _translate);
    if (_inverse) {
        _translate = - _translate;
    }
}

void StupeflixStackedMatrixElement::update(float t) {
    osg::Matrixf matrix;
    _sampler->getValueAt(t, matrix);
    if (_inverse) {
        _matrix = osg::Matrixf::inverse(matrix);
    } else {
        _matrix = matrix;
    }
}

void StupeflixStackedScaleElement::update(float t) {
    _sampler->getValueAt(t, _scale);
    if (_inverse) {
        for(int i = 0; i < 3; i++) {
            if (_scale[i] != 0) {
                _scale[i] = 1.0 / _scale[i];
            } else {
                _scale[i] = HUGE_VAL;
            }
        }
    }
}

void StupeflixStackedRotateAxisElement::update(float t) {
    osg::Vec4 value;
    _sampler->getValueAt(t, value);
    _axis =  osg::Vec3(value[1], value[2], value[3]);
    _angle = value[0]/ 180.0 * osg::PI;
    if (_inverse) {
        _angle = - _angle;
    }
}

void  StupeflixStackedQuaternionElement::update(float t) {
    _sampler->getValueAt(t, _quaternion);
    if (_inverse) {
        _quaternion = _quaternion.inverse();
    }
}
