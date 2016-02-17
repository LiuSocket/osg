//
//  AssetTimingsVisitor.cpp
//  StupeflixStudio
//
//  Created by Francois Lagunas on 09/04/13.
//  Copyright (c) 2013 Stupeflix. All rights reserved.
//

#include <osgStupeflix/TimeGroup>
#include <osgStupeflix/RenderOnce>
#include <osgStupeflix/AssetTimingsVisitor>
#include <osg/Notify>
#include <osg/Node>
#include <osg/Texture>
#include <osg/Geode>
#include <osg/ImageSequence>
#include <osg/Drawable>

#include <stdexcept>
#include <osgUtil/CullVisitor>

#include <cstdio> // printf

#include <algorithm> // For min / max

using namespace osgStupeflix;


AssetTimingsVisitor::AssetTimingsVisitor(AssetTimingsCallback callback,  void* callbackData):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{

    _callback = callback;
    _callbackData = callbackData;
    setTraversalMask(~0x0);
    setNodeMaskOverride(~0x0);
    _startTime = 0.0;
    _stopTime = HUGE_VAL;
}

void AssetTimingsVisitor::apply(osg::StateSet* stateset) {
    if (!stateset) {
        return;
    }
    for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i) {
        osg::StateAttribute* stateAttribute = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);
        
        osg::Texture* texture = dynamic_cast<osg::Texture*>(stateAttribute);
        
        if (texture) {
            osg::Image* image = texture->getImage(0); // 0 is the face number
            if (image) {
                osg::ImageSequence* imageSeq = dynamic_cast<osg::ImageSequence*> (image);
                if (imageSeq) {
                    std::vector<std::string> filenames = imageSeq->getFileNames();

                    for (int i=0; i < filenames.size(); i++) {
                        _callback(filenames[i], _startTime, _stopTime, _callbackData);
                    }
                }

                std::string className = std::string(image->className());

                //                printf("class name is %s", className.c_str());
                if (className.compare("IOSVideoImageStream") == 0) {
                    _callback(image->getFileName(), _startTime, _stopTime, _callbackData);
                    //                    printf("file name is %s", image->getFileName().c_str());
                }
            }
        }
    }
}


void AssetTimingsVisitor::apply(osg::Node& node)
{
    TimeGroup* timeGroup = dynamic_cast<TimeGroup*>(&node);
    double startTimeSave = _startTime;
    double stopTimeSave = _stopTime;

    if (timeGroup) {
        _startTime = std::max(_startTime, timeGroup->getStartTime());
        _stopTime = std::min(_stopTime, timeGroup->getStartTime() + timeGroup->getDuration());
    }

    if (_startTime <= _stopTime)
    {
        osg::Geode* geode = dynamic_cast<osg::Geode*>(&node);

        if (geode) {
            for(unsigned int i=0; i< geode->getNumDrawables(); ++i) {
                osg::Drawable* drawable = geode->getDrawable(i);
                apply(drawable->getStateSet());
            }
        }

        apply(node.getStateSet());

        traverse(node);
    }
    _startTime = startTimeSave;
    _stopTime = stopTimeSave;
}


