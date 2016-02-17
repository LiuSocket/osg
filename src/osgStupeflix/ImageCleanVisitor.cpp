/*  -*-c++-*-
 *  Copyright (C) 2010 Stupeflix
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/TimeGroup>
#include <osgStupeflix/RenderOnce>
#include <osg/Notify>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/ImageSequence>
#include <osg/Drawable>
#include <osgStupeflix/ImageCleanVisitor>
#include <osgStupeflix/ImageStream>
#include <stdexcept>
#include <osgUtil/CullVisitor>
#include <osgStupeflix/MarkActiveNodeVisitor>

#include <cstdio> // printf

using namespace osgStupeflix;

#define TOCLEAN 0x2
#define TOKEEP 0x1

// Helper for resetting cameras that went offscreen and then back : without this, the camera won't be reactivated correctly
class CameraResetCallback: public osg::NodeCallback
{
public:
    CameraResetCallback():m_resetFlag(true){}
    void operator() (osg::Node* node, osg::NodeVisitor* nv)
    {
        if (m_resetFlag)
        {
            osg::Camera* fboCam = dynamic_cast<osg::Camera*>( node );
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);

            if ( fboCam && cv)
            {
                cv->getCurrentRenderBin()->getStage()->setFrameBufferObject(NULL);
                cv->getCurrentRenderBin()->getStage()->setCameraRequiresSetUp( true );
                m_resetFlag = false;
            }

        }
        traverse(node,nv);
    }
    void reset(){m_resetFlag = true;}
private:
    bool m_resetFlag;
};



ImageCleanVisitor::ImageCleanVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    setTraversalMask(~0x0);
    _time = 0;
}

void ImageCleanVisitor::setCurrentTime(double time)
{
    _time = time;
    _cleaning = false;
    _textureStatus.clear();
    _cameraStatus.clear();
}

void ImageCleanVisitor::cleanupTexture(osg::Texture2D* texture) {
    if (texture) {
        osg::Image* image = texture->getImage();
        if (image == NULL) {
            return;
        }
        //osg::notify(osg::INFO) << "osgStupeflix::ImageCleanVisitor got texture " << image << " " << typeid(image).name() << std::endl;

        osg::ImageSequence* imageSeq = dynamic_cast<osg::ImageSequence*> (image);
        if (imageSeq) {
            //osg::notify(osg::INFO) << "osgStupeflix::ImageCleanVisitor got image seq " << std::endl;

            osg::Image* subImage = imageSeq->getImage(0);
            if (subImage) {

                //osg::notify(osg::INFO) << "osgStupeflix::ImageCleanVisitor got sub image " << std::endl;
                //                std::string filename = imageSeq->getFileNames()[0];
                imageSeq->reset();

                //osg::notify(osg::INFO) << "osgStupeflix::ImageCleanVisitor image cleaned name=" << filename << std::endl;
            }
        } else {
            osgStupeflix::ImageStream* imageStream = dynamic_cast<osgStupeflix::ImageStream*> (image);
            if (imageStream) {
                //osg::notify(osg::INFO) << "osgStupeflix::ImageCleanVisitor video cleaned name=" << imageStream->getFileName() << std::endl;

                imageStream->quit();
            } else {
                //osg::notify(osg::INFO) << "osgStupeflix::ImageCleanVisitor video not cleaned name=" << image->getFileName() << std::endl;

            }
        }
        texture->dirtyTextureObject();
        texture->releaseGLObjects();
    }
}

void ImageCleanVisitor::cleanupCamera(osg::Camera* camera) {
    if (camera) {
        // We have to clean up cameras and setup a reset callback to reactivate them later if needed
        try {
            osg::Camera::BufferAttachmentMap& attach_map = camera->getBufferAttachmentMap();
            if (attach_map.count(osg::Camera::COLOR_BUFFER) != 0) {
                osg::Camera::Attachment& attach = attach_map.at(osg::Camera::COLOR_BUFFER);
                osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(attach._texture.get());

                if (texture) {
                    //                    texture->dirtyTextureObject();
                    //                    texture->releaseGLObjects();
                }

                osg::NodeCallback* nodeCallback = camera->getCullCallback();
                CameraResetCallback* callback;

                if (nodeCallback) {
                    callback = dynamic_cast<CameraResetCallback*>(nodeCallback);
                } else {
                    callback = new CameraResetCallback();
                    camera->setCullCallback(callback);
                }

                callback->reset();
            }
        } catch (const std::out_of_range& e) {
            // This camera is not a rtt : no COLOR_BUFFER attachement
        }
    }
}


void ImageCleanVisitor::markTexture(osg::Texture2D* texture) {
    if (!texture) {
        return;
    }
    if (_cleaning) {
        _textureStatus[texture] |= TOCLEAN;
    } else {
        _textureStatus[texture] |= TOKEEP;
    }
}

void ImageCleanVisitor::markCamera(osg::Camera* camera) {
    if (!camera) {
        return;
    }
    if (_cleaning) {
        _cameraStatus[camera] |= TOCLEAN;
    } else {
        _cameraStatus[camera] |= TOKEEP;
    }
}

void ImageCleanVisitor::apply(osg::StateSet* stateset, std::string name) {
    if (!stateset) {
        return;
    }
    for(unsigned int i=0; i< stateset->getNumTextureAttributeLists(); ++i) {
        osg::StateAttribute* stateAttribute = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);
        osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateAttribute);
        if (texture) {
            markTexture(texture);
        }
    }
}


void ImageCleanVisitor::apply(osg::Node& node)
{
    TimeGroup* group = dynamic_cast<TimeGroup*>(&node);
    bool cleaningSave = _cleaning;
    RenderOnce* renderOnce = dynamic_cast<RenderOnce*>(&node);

    if (group) {
        if (group->getNodeMask() == 0x0) {
            // The group was not active , and is still not, nothing to clean up
            // This should not happen, as we don't override the traversal mask
            return;
        } else if (group->getNodeMask() == WASACTIVE) {
            _cleaning = true;
            // Clean the flag
            group->setNodeMask(0x0);
        } else {
            _cleaning = false;
        }
    }

    if (renderOnce){
        if (!_cleaning) {
            // We are not cleaning because of time, but maybe we should because of renderOnce ?
            if(renderOnce->needCleaning(_time)) {
                // Force cleaning !
                _cleaning = true;
            }
        } else {
            // We are cleaning everything, including the texture that should be at the same level as renderOnce
            renderOnce->reset();
        }
    }


    osg::Geode* geode = dynamic_cast<osg::Geode*>(&node);

    if (geode) {
        for(unsigned int i=0; i< geode->getNumDrawables(); ++i) {
            osg::Drawable* drawable = geode->getDrawable(i);
            apply(drawable->getStateSet(), "");
        }
    }
    apply(node.getStateSet(), "");

    // Check if this is a camera
    osg::Camera* camera = dynamic_cast<osg::Camera*>(&node);

    markCamera(camera);

    traverse(node);
    _cleaning = cleaningSave;


}

void ImageCleanVisitor::cleanupFinish() {
    for(std::map<osg::Texture2D*, osg::Node::NodeMask> ::const_iterator it = _textureStatus.begin(); it != _textureStatus.end(); ++it )
    {
        osg::Texture2D* key = it->first;
        osg::Node::NodeMask mask = it->second;

        if (mask == TOCLEAN) {
            cleanupTexture(key);
        }
    }
    for(std::map<osg::Camera*, osg::Node::NodeMask> ::const_iterator it = _cameraStatus.begin(); it != _cameraStatus.end(); ++it )
    {
        osg::Camera* key = it->first;
        osg::Node::NodeMask mask = it->second;

        if (mask == TOCLEAN) {
            cleanupCamera(key);
        }
    }
}


