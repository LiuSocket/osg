//
//  CaptureCallBack.mm
//  StupeflixDemo
//
//  Created by Guenole on 08/10/2012.
//  Copyright (c) 2012 Stupeflix. All rights reserved.
//

#include <osgStupeflix/CaptureCallBack>
#include <iostream>

using namespace osgStupeflix;

// Callback: read image and post notification to delegate
void CaptureCallback::operator () (osg::RenderInfo& renderInfo) const {
    osg::GraphicsContext* graphicsContext = renderInfo.getState()->getGraphicsContext();
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    osg::ref_ptr<ContextData>& contextData = _contextDataMap[graphicsContext];
    if (!contextData) {
        contextData = new ContextData(graphicsContext, _delegate);
        _contextDataMap[graphicsContext] = contextData;
    }
    contextData->readImage();
}



void CaptureCallback::ContextData::readImage()
{
    int width = _graphicsContext->getTraits()->width;
    int height = _graphicsContext->getTraits()->height;

    if (width!=_width || _height!=height)
    {
        _width = width; 
        _height = height;
    }
    

    if (_delegate)
    {
 
        VideoBuffer *buffer = _delegate->getVideoBuffer();

        GLint internalFormat = GL_BGRA;     // This is fine for iOS

#ifdef ANDROID                              // Depending in the platform / android version, the pixel format will change
        glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &internalFormat);
        if (internalFormat == GL_RGB)
            internalFormat = GL_RGBA;
#endif
        
        buffer->pixelFormat = internalFormat;


        size_t bpp = buffer->bpp();
        size_t bpr = buffer->bpr();

        GLubyte *pixelBufferData = (GLubyte *)buffer->data();

        glReadPixels(0, 0, width, height, internalFormat, GL_UNSIGNED_BYTE, pixelBufferData);

        
        if (bpr != width*bpp){

            // There is some padding expected in the buffer, let's move stuff around
            for (int y = height-1; y > 0; --y){

                memmove(pixelBufferData + y * bpr, pixelBufferData + y * width * bpp, width * bpp);

            }
            
        } 
        
        _delegate->didCaptureImage();
        
    }
}


