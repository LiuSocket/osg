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

#include <osgStupeflix/ScreenCapture>
//#include <osgStupeflix/WindowCaptureCallbackBase>
#include <cstring>
#include <sstream>

using namespace osgStupeflix;

namespace osgStupeflix {

    WindowCaptureCallbackImage::WindowCaptureCallbackImage(osg::Image* img, unsigned int startFrame = 0) : osg::Camera::DrawCallback(),
                                                                               _frame(startFrame),
                                                                               _image(img)
    {}

    std::string WindowCaptureCallbackImage::getImageData(unsigned int frame) {
        if (frame != _frame-1) {
            osg::notify(osg::WARN) << "asked " << frame << " current available " << (_frame-1) << std::endl;
        }
        std::string str((char*)_image->data(), _image->getImageSizeInBytes());
        return str;
    }

    void WindowCaptureCallbackImage::operator () (osg::RenderInfo& renderInfo) const {
        _image->flipVertical();
        _frame++;
    }

}

ScreenCapture::ScreenCapture(unsigned int startFrame, osgViewer::Viewer* viewer) :
    _startFrame(startFrame),
    _viewer(viewer)
{}


std::string ScreenCapture::getImageData(unsigned int frame)
{
    return _callback->getImageData(frame);
}


bool ScreenCapture::installToCamera(osg::Camera* camera, FramePosition position, bool display)
{
    osg::Viewport* vp = camera->getViewport();
    if (!vp) {
        osg::notify(osg::WARN) << "no viewport attached to camera, cant get size" << std::endl;
        return false;
    }

    unsigned int width = vp->width();
    unsigned int height = vp->height();

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->width = width;
    traits->height = height;
    traits->doubleBuffer = true;
    
    if (display) {
        // disable vsync on viewer camera
        traits->vsync = false;
        osg::ref_ptr<osg::GraphicsContext> viewerGraphicContext = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (viewerGraphicContext.valid()) {
            _viewer->getCamera()->setGraphicsContext(viewerGraphicContext);
        } else {
            osg::notify(osg::WARN) << "Unable to create viewer graphic context" << std::endl;
            return false;
        }
    } else {
        // off-screen rendering
        traits->pbuffer = true;
        // alpha in buffer
        traits->alpha = 8;

        osg::ref_ptr<osg::GraphicsContext> offscreenGraphicContext = osg::GraphicsContext::createGraphicsContext(traits.get());
        if (offscreenGraphicContext.valid()) {
            camera->setGraphicsContext(offscreenGraphicContext.get());
            GLenum buffer = offscreenGraphicContext->getTraits()->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer(buffer);
            camera->setReadBuffer(buffer);
        } else {
            osg::notify(osg::WARN) << "Unable to create offscreen graphic context" << std::endl;
            return false;
        }
    }

    osg::Image* img = new osg::Image;
    img->allocateImage(width, height, 1, GL_RGB, GL_UNSIGNED_BYTE);
    osg::notify(osg::INFO) << "Capture " << width << " x " << height << std::endl;
    _callback = new WindowCaptureCallbackImage(img, _startFrame);
    WindowCaptureCallbackImage* callback = _callback.get();
        
    camera->attach(osg::Camera::COLOR_BUFFER, img, 0, 0);

    if (position == START_FRAME) {
        camera->setInitialDrawCallback(callback);
    } else {    
        camera->setFinalDrawCallback(callback);
    }
    return true;
}
