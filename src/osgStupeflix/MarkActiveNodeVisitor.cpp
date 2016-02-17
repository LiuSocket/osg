/*  -*-c++-*-
 *  Copyright (C) 2010 Stupeflix
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/TimeGroup>
#include <osgStupeflix/RenderOnce>
#include <osgStupeflix/ImageStream>
#include <osg/Notify>
#include <osg/Node>
#include <osg/Texture>
#include <osg/Geode>
#include <osg/ImageSequence>
#include <osg/Drawable>
#include <osg/Notify>
#include <osgStupeflix/MarkActiveNodeVisitor>
#include <osgDB/FileUtils>

using namespace osgStupeflix;


ResetAllNodeVisitor::ResetAllNodeVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    setTraversalMask(ACTIVE|PREFETCH|WASACTIVE);
}


void ResetAllNodeVisitor::apply(osg::Group& node)
{
    TimeGroup* timeGroup = dynamic_cast<TimeGroup*>(&node);
    RenderOnce* renderOnce = dynamic_cast<RenderOnce*>(&node);
    if (timeGroup || renderOnce) {
        osg::Node::NodeMask fullNodeMask = node.getNodeMask();
        osg::Node::NodeMask mask = fullNodeMask & (ACTIVE | PREFETCH);
        // remaining should be 0 except for newly added nodes
        osg::Node::NodeMask remaining = fullNodeMask & UNUSED_BITS;
        if ((mask != 0) && (remaining == 0x0)) {
#if 0
            if (timeGroup) {
                NSLog(@"Resetting [%f, %f] to WASACTIVE %d, %d, %d",
                      timeGroup->getStartTime(),
                      timeGroup->getStartTime()+ timeGroup->getDuration(),
                      fullNodeMask, mask, remaining);
            }
#endif
            node.setNodeMask(WASACTIVE);
        } else {
#if 0
            if (timeGroup) {
                NSLog(@"Resetting [%f, %f] to 0x0 %d, %d, %d",
                      timeGroup->getStartTime(),
                      timeGroup->getStartTime()+ timeGroup->getDuration(),
                      fullNodeMask, mask, remaining);
            }
#endif
            node.setNodeMask(0x0);
        }
    }
    // Optimization : only those nodes were potentially active
    traverse(node);
}

MarkActiveNodeVisitor::MarkActiveNodeVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    setNodeMaskOverride(~0x0);
    _time = -1;
}

void MarkActiveNodeVisitor::setCurrentTime(double time)
{
    _time = time;
}

void MarkActiveNodeVisitor::setNodeMask(osg::Node* node, osg::Node::NodeMask mask) {
    mask = mask | node->getNodeMask();
    node->setNodeMask(mask);
}

void MarkActiveNodeVisitor::apply(osg::StateSet* stateset) {
}

void MarkActiveNodeVisitor::apply(osg::Node& node)
{
    TimeGroup* timeGroup = dynamic_cast<TimeGroup*>(&node);
    RenderOnce* renderOnce = dynamic_cast<RenderOnce*>(&node);
    osg::Node::NodeMask wasActiveNodeMask = node.getNodeMask() & WASACTIVE;
    float prefetchTime = PREFETCH_TIME;

    if (timeGroup) {
        if (_time >= timeGroup->getStartTime() && _time < (timeGroup->getStartTime() + timeGroup->getDuration())) {
            // ACTIVE
            setNodeMask(timeGroup, ACTIVE | wasActiveNodeMask);
            traverse(node);
        } else if (_time >= (timeGroup->getStartTime() - prefetchTime) && _time < timeGroup->getStartTime()) {
            // PREFETCHING
            setNodeMask(timeGroup, PREFETCH | wasActiveNodeMask);
            traverse(node);
        } else {
            // INACTIVE
            setNodeMask(timeGroup, 0x0 | wasActiveNodeMask);
        }
    }
    else if (renderOnce != NULL)
    {
        osg::Node::NodeMask nodeMask = renderOnce->nodeMask(_time);

        setNodeMask(renderOnce, nodeMask | wasActiveNodeMask);

        if (nodeMask & (ACTIVE | PREFETCH)) {
            traverse(node);
        }
    } else {
        traverse(node);
    }

    return;
}

CheckActiveNodeVisitor::CheckActiveNodeVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{
    setTraversalMask(ACTIVE | PREFETCH);
    _time = -1;
}

void CheckActiveNodeVisitor::setCurrentTime(double time)
{
    _time = time;
    _imageWasNotLoaded = false;
    _fileDidNotExist = false;
    _currentMask =  0xffffffff;
}

void CheckActiveNodeVisitor::apply(osg::StateSet* stateset) {
    if (!stateset) {
        return;
    }
    for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i) {
        osg::StateAttribute* stateAttribute = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);

        osg::Texture* texture = dynamic_cast<osg::Texture*>(stateAttribute);

        if (texture) {
            //            osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor got texture nb=" << i  << std::endl;
            osg::Image* image = texture->getImage(0); // 0 is the face number
            if (image)  {
                //osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor image was found: " << image->getFileName()  << std::endl;
            }
            osg::ImageSequence* imageSeq = dynamic_cast<osg::ImageSequence*> (image);
            if (imageSeq)  {
                //std::string filename = imageSeq->getFileNames()[0];
                //osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor imageSequence was found: " << filename << std::endl;
            }

            if (imageSeq) {
                if ((_currentMask & (PREFETCH | ACTIVE)) != 0) {
                    if (imageSeq != NULL) {
                        osg::Image* subImage = imageSeq->getImage(0);

                        if ((_currentMask & ACTIVE) != 0) {
                            if (! subImage) {
                                _imageWasNotLoaded = true;
                                if (osg::getNotifyLevel() >= osg::INFO) {
                                    //osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor image was not loaded: " << imageSeq->getFileNames()[0]  << std::endl;
                                }
                            }
                        }
                        std::string filename;

                        if (imageSeq != NULL) {
                            filename = imageSeq->getFileNames()[0];
                        }

                        std::string fileprotocol("file://");
                        if (filename.substr(0, fileprotocol.length()) == fileprotocol) {
                            int end = filename.find("?");
                            if (end == std::string::npos) {
                                end = filename.length();
                            }
                            filename = filename.substr(fileprotocol.length(), end - fileprotocol.length());
                        } else if (filename.substr(0, 1) != std::string("/")) {
                            filename = "";
                        }

                        if (filename.length() != 0 &&  !osgDB::fileExists(filename)) {
                            // The file should exists, while prefetching or active
                            _fileDidNotExist = true;
                            if (osg::getNotifyLevel() >= osg::INFO) {
                                //osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor file does not exists: " << filename  << std::endl;
                            }
                        }
                        if (_fileDidNotExist) {
                            break;
                        }
                    }
                }
            }

            osgStupeflix::ImageStream* imageStream = dynamic_cast<osgStupeflix::ImageStream*> (image);
            if (imageStream) {
                if ((_currentMask & ACTIVE) != 0) {
                    if (!imageStream->ready(_time)) {
                        //osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor imagestream was not ready: " << imageStream->getFileName() << std::endl;
                        _imageWasNotLoaded = true;
                    }
                }
            }
        }
    }
}


void CheckActiveNodeVisitor::apply(osg::Node& node)
{
    osg::Node::NodeMask currentMaskSave = _currentMask;

    _currentMask &= node.getNodeMask();

    if (_fileDidNotExist) {
        _currentMask = currentMaskSave;
        return;
    }

    osg::Geode* geode = dynamic_cast<osg::Geode*>(&node);

    if (geode) {
        //        osg::notify(osg::INFO) << "osgStupeflix: CheckActiveNodeVisitor geode name=" << name  << std::endl;
        for(unsigned int i=0; i< geode->getNumDrawables(); ++i) {
            osg::Drawable* drawable = geode->getDrawable(i);
            apply(drawable->getStateSet());
        }
    }

    apply(node.getStateSet());

    if (_fileDidNotExist) {
        _currentMask = currentMaskSave;
        return;
    }

    traverse(node);
    _currentMask = currentMaskSave;
}

