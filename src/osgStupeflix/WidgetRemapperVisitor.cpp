/*  -*-c++-*-
 *  Copyright (C) 2010 Stupeflix
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/WidgetRemapperVisitor>
#include <osgStupeflix/TimeGroup>
#include <osgStupeflix/Animation>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/ImageSequence>
#include <osg/Drawable>
#include <osg/ImageStream>
#include <osgText/Text>
#include <sstream>


#define IMAGE_NAME_PREFIX std::string("IMAGE_")
#define TEXT_NAME_PREFIX std::string("TEXT_")

using namespace osgStupeflix;

WidgetDurationFinder::WidgetDurationFinder():
  osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
  _duration(-1)
{
}


void WidgetDurationFinder::apply(osg::Node& node) {
    TimeGroup* group = dynamic_cast<TimeGroup*>(&node);

    if (group) {
        _duration =  group->getDuration();
    } else {
      traverse(node);
    }
}


WidgetRemapperVisitor::WidgetRemapperVisitor(float timeShift, float timeStretch, std::string pathPrefix, std::string suffix,
                                             std::vector<osg::StateSet*> statesets, std::vector<std::string> texts):
  osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
  _timeShift(timeShift),
  _timeStretch(timeStretch),
  _pathPrefix(pathPrefix),
  _suffix(suffix),
  _statesets(statesets),
  _texts(texts),
  _currentStartTime(0.0)
{
}

void WidgetRemapperVisitor::apply(osg::StateSet* stateset) {
    apply(stateset, true);
}

void WidgetRemapperVisitor::apply(osg::StateSet* stateset, bool fileremap) {
    if (!stateset) {
        return;
    }
    osgStupeflix::stateSetTimeStretch(stateset, _timeShift, _timeStretch);

    for(unsigned int i=0; i< stateset->getNumTextureAttributeLists(); ++i) {
        osg::StateAttribute* stateAttribute = stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE);
        osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateAttribute);

        if (texture) {
            osg::Image* image = texture->getImage();

            if (fileremap) {
                osg::ImageSequence* imageSeq = dynamic_cast<osg::ImageSequence*> (image);
                if (imageSeq) {
                    osg::ImageSequence::FileNames& filenames = imageSeq->getFileNames();
                    for(int i =0; i < filenames.size() ; i++) {
                        std::string s = filenames[i];
                        
                        if (s.compare(0, _pathPrefix.size(), _pathPrefix) != 0)
                        {
                            std::string newname = _pathPrefix + s;
                            
                            if (_suffix.length() != 0) {
                                int lastindex = newname.find_last_of("."); 
                                if (lastindex != std::string::npos) {
                                    newname = newname.substr(0, lastindex); 
                                }
                                
                                newname += _suffix;
                            }
                            
                            filenames[i] = newname;
                        }
                    }
                }
            }
            osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*> (image);
            
            if (imageStream) {
                osg::notify(osg::INFO) << "WidgetRemapper : imageStream is ok"  << std::endl;                
                imageStream->setReferenceTime(_currentStartTime);
            } else {
                osg::notify(osg::INFO) << "WidgetRemapper : imageStream is no ok"  << std::endl;                
            }


        }
    }

}

int WidgetRemapperVisitor::getIndexFromName(std::string name, std::string prefix)
{
    int replaceIndex = -1;
    if (name.length() >= prefix.length()) {
        if (name.substr(0, prefix.length()).compare(prefix) == 0) {
            std::string remaining = name.substr(prefix.size(), name.length());
            std::istringstream(remaining) >> replaceIndex;
            // INDEX ARE 0 based
            replaceIndex -= 1;
        }
    }
    return replaceIndex;
}


void WidgetRemapperVisitor::apply(osg::Node& node) {
    float currentStartTimeSave = _currentStartTime;

    TimeGroup* group = dynamic_cast<TimeGroup*>(&node);

    if (group) {
        float startTime = group->getStartTime();
        float duration = group->getDuration();
        float newStartTime = startTime * _timeStretch + _timeShift;
        
        _currentStartTime = newStartTime;         
        group->setStartTime(newStartTime);
        group->setDuration(duration * _timeStretch);
    }


    osgStupeflix::nodeTimeStretch(&node, _timeShift, _timeStretch);
      
    osg::Geode* geode = dynamic_cast<osg::Geode*>(&node);

    if (geode) {
        for(unsigned int i=0; i< geode->getNumDrawables(); ++i) {
            osg::Drawable* drawable = geode->getDrawable(i);
            osgText::Text* textNode = dynamic_cast<osgText::Text*>(drawable);
            std::string name = drawable->getName();
            int replaceIndex = getIndexFromName(name, IMAGE_NAME_PREFIX);

            if (replaceIndex >= 0 && replaceIndex < _statesets.size()) {
                osg::StateSet* stateset = _statesets[replaceIndex];
                if (stateset != NULL) {
                    drawable->setStateSet(stateset);
                }
                apply(drawable->getStateSet(), false);
            } else {
                apply(drawable->getStateSet(), true);
            }
            // Replace texts
            replaceIndex = getIndexFromName(name, TEXT_NAME_PREFIX);
            if (replaceIndex >= 0 && replaceIndex < _texts.size() && textNode != NULL)
            {
                //                osg::notify(osg::NOTICE) << "Widget: found text, font = " << textNode->getFont()  << std::endl;
                textNode->setText(_texts[replaceIndex]);
            }
        }
    }
    apply(node.getStateSet(), true);

    traverse(node);
    _currentStartTime = currentStartTimeSave;
}


void osgStupeflix::widgetRemap(osg::Node* node, 
                               float startTime, 
                               float duration, 
                               std::string pathPrefix,
                               std::string pathSuffix,
                               std::vector<osg::StateSet*> statesets,
                               std::vector<std::string> texts)
{
  WidgetDurationFinder finder = WidgetDurationFinder();
  node->accept(finder);

  osg::notify(osg::NOTICE) << "Widget: found duration : " <<  finder.getDuration() << " " << duration << " " << statesets.size()  << std::endl;

  for (int i = 0 ; i < texts.size(); i++ ) {
      osg::notify(osg::NOTICE) << texts[i] << std::endl;
  }

  // TEMPORARY : use a config to choose this
  WidgetRemapperVisitor visitor = WidgetRemapperVisitor(startTime, duration / finder.getDuration(), pathPrefix, pathSuffix, statesets, texts);
  node->accept(visitor);
}
