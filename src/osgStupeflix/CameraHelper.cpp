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


#include <osgStupeflix/CameraHelper>
#include <stdexcept>

using namespace osgStupeflix;

osg::Texture2D* osgStupeflix::getColorBufferFromCamera(osg::Node* camera_) {
    try {
        osg::Camera* camera = dynamic_cast<osg::Camera*>(camera_);
        if (camera == NULL) {
            return NULL;
        }
        osg::Camera::BufferAttachmentMap& attach_map = camera->getBufferAttachmentMap();
        osg::Camera::Attachment& attach = attach_map.at(osg::Camera::COLOR_BUFFER);
        osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(attach._texture.get());

        return texture;
    } catch (const std::out_of_range& e) {
        return NULL;
    }
}
