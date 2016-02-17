/* OpenSceneGraph example, osgpick.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* osgpick sample
* demonstrate use of osgUtil/PickVisitor for picking in a HUD or
* in a 3d scene,
*/

#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/CompositeViewer>

#include <osgGA/TerrainManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osg/Material>
#include <osg/Geode>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osg/Camera>
#include <osg/io_utils>
#include <osg/ShapeDrawable>
#include <osg/ImageSequence>
#include <osgStupeflix/TimeGroup>
#include <osgStupeflix/ImageColor>
#include <osgText/Text>
#include <osgStupeflix/TexMat>
#include <osgStupeflix/Animation>

#include <sstream>

static osg::MatrixTransform* matrixTransformTest() {
    osg::MatrixTransform* matrixTransform0 = new osg::MatrixTransform();

    osgAnimation::Vec3KeyframeContainer* keysMT0 = new osgAnimation::Vec3KeyframeContainer();

    osgAnimation::Vec3Keyframe keysMT00 = osgAnimation::Vec3Keyframe(0.2, osg::Vec3(1.0, 1.0, 0.0));
    osgAnimation::Vec3Keyframe keysMT01 = osgAnimation::Vec3Keyframe(1.8, osg::Vec3(1.2, 0.8, 0.7));
    keysMT0->push_back(keysMT00);
    keysMT0->push_back(keysMT01);

    osgStupeflix::setAnimationScaleVec3(matrixTransform0, keysMT0);

    return matrixTransform0;
}


static void texMatTest(osg::StateSet* stateset)
{
    osgStupeflix::TexMat* texMat = new osgStupeflix::TexMat(1);

    osg::Matrix* mat = new osg::Matrix();
    mat->makeScale(1.0, 2.0, 1.1);
    texMat->setMatrix(*mat);

    osgStupeflix::UpdateTexMat* updateTexMat = osgStupeflix::getOrCreateTexMatUpdateCallback(texMat);

    osgAnimation::Vec3KeyframeContainer* keyframeContainer0 = new osgAnimation::Vec3KeyframeContainer();

    osgAnimation::Vec3Keyframe keyframe00 = osgAnimation::Vec3Keyframe(0.2, osg::Vec3(1.0, 1.0, 0.0));
    osgAnimation::Vec3Keyframe keyframe01 = osgAnimation::Vec3Keyframe(1.8, osg::Vec3(1.2, 0.8, 0.7));
    keyframeContainer0->push_back(keyframe00);
    keyframeContainer0->push_back(keyframe01);

    updateTexMat->addTranslate(keyframeContainer0);

    osgAnimation::Vec3KeyframeContainer* keyframeContainer1 = new osgAnimation::Vec3KeyframeContainer();

    osgAnimation::Vec3Keyframe keyframe10 = osgAnimation::Vec3Keyframe(0.2, osg::Vec3(1.0, 1.0, 0.0));
    osgAnimation::Vec3Keyframe keyframe11 = osgAnimation::Vec3Keyframe(1.8, osg::Vec3(1.2, 0.8, 0.7));
    keyframeContainer1->push_back(keyframe10);
    keyframeContainer1->push_back(keyframe11);

    updateTexMat->addTranslate(keyframeContainer0);

    stateset->setTextureAttribute(0, texMat);

    osg::Texture2D* texture = new osg::Texture2D();
    osg::ImageSequence* imageSequence = new osg::ImageSequence();

    imageSequence->addImageFile("/tmp/coucou.png");

    texture->setImage(imageSequence);

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

}


static void uniformAnimationTest(osg::StateSet* stateset)
{
    // Uniform Vec4
    osg::Uniform* uniform4 = new osg::Uniform("LightPosition4", osg::Vec4(0.0, 0.0, 0.0f, 0.0f));
    stateset->addUniform(uniform4);

    osgAnimation::Vec4KeyframeContainer* keyframeContainer4 = new osgAnimation::Vec4KeyframeContainer();

    osgAnimation::Vec4Keyframe keyframe40 = osgAnimation::Vec4Keyframe(0.2, osg::Vec4(0.0, 0.0, 1.0, 0.0));
    osgAnimation::Vec4Keyframe keyframe41 = osgAnimation::Vec4Keyframe(1.8, osg::Vec4(0.0, 0.0, 1.2, 0.7));
    keyframeContainer4->push_back(keyframe40);
    keyframeContainer4->push_back(keyframe41);

    osgStupeflix::setAnimationUniformVec4(uniform4, keyframeContainer4);

    // Uniform Vec3
    osg::Uniform* uniform3 = new osg::Uniform("LightPosition3", osg::Vec3(0.0f, 0.0f, 4.0f));
    stateset->addUniform(uniform3);

    osgAnimation::Vec3KeyframeContainer* keyframeContainer2 = new osgAnimation::Vec3KeyframeContainer();

    osgAnimation::Vec3Keyframe keyframe20 = osgAnimation::Vec3Keyframe(0.2, osg::Vec3(1.0, 1.0, 0.0));
    osgAnimation::Vec3Keyframe keyframe21 = osgAnimation::Vec3Keyframe(1.8, osg::Vec3(1.2, 0.8, 0.7));
    keyframeContainer2->push_back(keyframe20);
    keyframeContainer2->push_back(keyframe21);

    osgStupeflix::setAnimationUniformVec3(uniform3, keyframeContainer2);

    // Uniform Vec2
    osg::Uniform* uniform2 = new osg::Uniform("LightPosition2", osg::Vec2(0.0f, 0.0f));
    stateset->addUniform(uniform2);

    osgAnimation::Vec2KeyframeContainer* keyframeContainer3 = new osgAnimation::Vec2KeyframeContainer();

    osgAnimation::Vec2Keyframe keyframe30 = osgAnimation::Vec2Keyframe(0.2, osg::Vec2(1.0, 0.0));
    osgAnimation::Vec2Keyframe keyframe31 = osgAnimation::Vec2Keyframe(1.8, osg::Vec2(1.2, 0.7));
    keyframeContainer3->push_back(keyframe30);
    keyframeContainer3->push_back(keyframe31);

    osgStupeflix::setAnimationUniformVec2(uniform2, keyframeContainer3);

    // Uniform float
    osg::Uniform* uniformfloat = new osg::Uniform("LightPositionFloat", 0.4);
    stateset->addUniform(uniformfloat);

    osgAnimation::FloatKeyframeContainer* keyframeContainerFloat = new osgAnimation::FloatKeyframeContainer();

    osgAnimation::FloatKeyframe keyframeFloat0 = osgAnimation::FloatKeyframe(0.2, 1.0);
    osgAnimation::FloatKeyframe keyframeFloat1 = osgAnimation::FloatKeyframe(1.8, 1.2);
    keyframeContainerFloat->push_back(keyframeFloat0);
    keyframeContainerFloat->push_back(keyframeFloat1);

    osgStupeflix::setAnimationUniformFloat(uniformfloat, keyframeContainerFloat);

    // Uniform float

    osg::Uniform* uniformdouble = new osg::Uniform("LightPositionDouble", 0.4);
    stateset->addUniform(uniformdouble);

    osgAnimation::DoubleKeyframeContainer* keyframeContainerDouble = new osgAnimation::DoubleKeyframeContainer();

    osgAnimation::DoubleKeyframe keyframeDouble0 = osgAnimation::DoubleKeyframe(0.2, 1.0);
    osgAnimation::DoubleKeyframe keyframeDouble1 = osgAnimation::DoubleKeyframe(1.8, 1.2);
    keyframeContainerDouble->push_back(keyframeDouble0);
    keyframeContainerDouble->push_back(keyframeDouble1);

    osgStupeflix::setAnimationUniformDouble(uniformdouble, keyframeContainerDouble);

 }


static void uniformBezierAnimationTest(osg::StateSet* stateset)
{
    // Uniform Vec4
    osg::Uniform* uniform4 = new osg::Uniform("LightPositionBezier4", osg::Vec4(0.0, 0.0, 0.0f, 0.0f));
    stateset->addUniform(uniform4);

    osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec4VariableSpeedCubicBezier>* keyframeContainer4;

    keyframeContainer4 = new osgAnimation::TemplateKeyframeContainer<osgAnimation::Vec4VariableSpeedCubicBezier>();

    osgAnimation::Vec4VariableSpeedCubicBezier keyframe40Data = osgAnimation::Vec4VariableSpeedCubicBezier(osg::Vec4(0.0, 0.0, 1.0, 0.0),
                                                                                                           osg::Vec4(0.0, -1.0, 1.0, 0.0),
                                                                                                           osg::Vec4(0.0, 1.0, 1.0, 0.0),
                                                                                                           osg::Vec2(0.2, 0.3),
                                                                                                           osg::Vec2(0.5,0.8));


    osgAnimation::TemplateKeyframe<osgAnimation::TemplateVariableSpeedCubicBezier<osg::Vec4f> > keyframe40 = osgAnimation::TemplateKeyframe<osgAnimation::TemplateVariableSpeedCubicBezier<osg::Vec4f> > (0.1, keyframe40Data);

    keyframeContainer4->push_back(keyframe40);
    keyframeContainer4->push_back(keyframe40);

    osgStupeflix::setAnimationUniformVariableSpeedCubicBezierVec4(uniform4, keyframeContainer4);

#if 0

    // Uniform Vec3
    osg::Uniform* uniform3 = new osg::Uniform("LightPosition3", osg::Vec3(0.0f, 0.0f, 4.0f));
    stateset->addUniform(uniform3);

    osgAnimation::Vec3KeyframeContainer* keyframeContainer2 = new osgAnimation::Vec3KeyframeContainer();

    osgAnimation::Vec3Keyframe keyframe20 = osgAnimation::Vec3Keyframe(0.2, osg::Vec3(1.0, 1.0, 0.0));
    osgAnimation::Vec3Keyframe keyframe21 = osgAnimation::Vec3Keyframe(1.8, osg::Vec3(1.2, 0.8, 0.7));
    keyframeContainer2->push_back(keyframe20);
    keyframeContainer2->push_back(keyframe21);

    osgStupeflix::setAnimationUniformVec3(uniform3, keyframeContainer2);

    // Uniform Vec2
    osg::Uniform* uniform2 = new osg::Uniform("LightPosition2", osg::Vec2(0.0f, 0.0f));
    stateset->addUniform(uniform2);

    osgAnimation::Vec2KeyframeContainer* keyframeContainer3 = new osgAnimation::Vec2KeyframeContainer();

    osgAnimation::Vec2Keyframe keyframe30 = osgAnimation::Vec2Keyframe(0.2, osg::Vec2(1.0, 0.0));
    osgAnimation::Vec2Keyframe keyframe31 = osgAnimation::Vec2Keyframe(1.8, osg::Vec2(1.2, 0.7));
    keyframeContainer3->push_back(keyframe30);
    keyframeContainer3->push_back(keyframe31);

    osgStupeflix::setAnimationUniformVec2(uniform2, keyframeContainer3);

    // Uniform float
    osg::Uniform* uniformfloat = new osg::Uniform("LightPositionFloat", 0.4);
    stateset->addUniform(uniformfloat);

    osgAnimation::FloatKeyframeContainer* keyframeContainerFloat = new osgAnimation::FloatKeyframeContainer();

    osgAnimation::FloatKeyframe keyframeFloat0 = osgAnimation::FloatKeyframe(0.2, 1.0);
    osgAnimation::FloatKeyframe keyframeFloat1 = osgAnimation::FloatKeyframe(1.8, 1.2);
    keyframeContainerFloat->push_back(keyframeFloat0);
    keyframeContainerFloat->push_back(keyframeFloat1);

    osgStupeflix::setAnimationUniformFloat(uniformfloat, keyframeContainerFloat);

    // Uniform float

    osg::Uniform* uniformdouble = new osg::Uniform("LightPositionDouble", 0.4);
    stateset->addUniform(uniformdouble);

    osgAnimation::DoubleKeyframeContainer* keyframeContainerDouble = new osgAnimation::DoubleKeyframeContainer();

    osgAnimation::DoubleKeyframe keyframeDouble0 = osgAnimation::DoubleKeyframe(0.2, 1.0);
    osgAnimation::DoubleKeyframe keyframeDouble1 = osgAnimation::DoubleKeyframe(1.8, 1.2);
    keyframeContainerDouble->push_back(keyframeDouble0);
    keyframeContainerDouble->push_back(keyframeDouble1);

    osgStupeflix::setAnimationUniformDouble(uniformdouble, keyframeContainerDouble);
#endif
 }

void timeGroupTest(osg::Group* group) {
    osgStupeflix::TimeGroup* group3 = new osgStupeflix::TimeGroup();

    group3->setDuration(2.0);
    group3->setStartTime(4.0);

    group3->setName("coucou stupeflix");

    group->addChild(group3);

    osgStupeflix::TimeGroup* group4 = new osgStupeflix::TimeGroup();

    group4->setDuration(2.4);
    group4->setStartTime(4.4);

    group4->setName("coucou stupeflix");


    group3->addChild(group4);
}


int main( int argc, char **argv )
{
    osg::MatrixTransform* transform  = (osg::MatrixTransform*) osgDB::readNodeFile("scene3.osgt");

    osgStupeflix::UpdateMatrixTransform* updateMatrix = osgStupeflix::getMatrixUpdateCallback(transform);
    osgAnimation::TypedSampler<osg::Vec4> * posSampler = updateMatrix->getAxisRotation();
    osg::Vec4 output;
    for (int i = 0; i < 300; i++) {
        float time = 0.52;
        posSampler->getValueAt(time, output);
        std::cout << "FINAL RESULT: " << time << " " << output << std::endl;
        break;
    }


    return 0;
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // Create a test scene with a camera that has a relative reference frame.
    osg::Group* group = new osg::Group();

#if 1
    osg::MatrixTransform* matrixTransform0 = matrixTransformTest();

    group->addChild(matrixTransform0);
    group = matrixTransform0;
#endif

    osg::StateSet* stateset = group->getOrCreateStateSet();

#if 1
    texMatTest(stateset);
#endif

#if 0
    uniformAnimationTest(stateset);
#endif


#if 0
    uniformBezierAnimationTest(stateset);
#endif

#if 0
    timeGroupTest(group);
#endif

    osg::Geode* sphere = new osg::Geode();
    sphere->setName("Sphere");
    sphere->addDrawable(new osg::ShapeDrawable(new osg::Sphere()));

    osg::Geode* cube = new osg::Geode();
    cube->setName("Cube");
    cube->addDrawable(new osg::ShapeDrawable(new osg::Box()));

    osg::Camera* camera = new osg::Camera();
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
    camera->setViewMatrix(osg::Matrix::translate(-2, 0, 0));

    camera->addChild(cube);

    osg::MatrixTransform* xform = new osg::MatrixTransform(osg::Matrix::translate(1, 1, 1));
    xform->addChild(camera);

    group->addChild(xform);

    group->addChild(sphere);

    osg::Node* scene = group;

    osgDB::writeNodeFile(*scene, "scene.osgt"); //, new osgDB::Options("Compressor=zlib"));

#if 1
    osg::Node* node = osgDB::readNodeFile("scene.osgt"); //, new osgDB::Options("Compressor=zlib"));

    osgDB::writeNodeFile(*node, "scene2.osgt");//, new osgDB::Options("Compressor=zlib"));
#endif
    return 0;

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!scene) scene = osgDB::readNodeFile("fountain.osgb");

    osg::ref_ptr<osgText::Text> updateText = new osgText::Text;

        osgViewer::Viewer viewer;


        // add all the camera manipulators
        {
            osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

            keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
            keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
            keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );

            unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
            keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

            std::string pathfile;
            char keyForAnimationPath = '5';
            while (arguments.read("-p",pathfile))
            {
                osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
                if (apm || !apm->valid())
                {
                    num = keyswitchManipulator->getNumMatrixManipulators();
                    keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                    ++keyForAnimationPath;
                }
            }

            keyswitchManipulator->selectMatrixManipulator(num);

            viewer.setCameraManipulator( keyswitchManipulator.get() );
        }

        // set the scene to render
        viewer.setSceneData(group);

        return viewer.run();

}
