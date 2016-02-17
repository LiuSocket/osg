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

#include <osgStupeflix/WindowCaptureCallback>
#include <OpenThreads/Thread>
#include <cstring>
#include <cassert>
#include <sstream>

using namespace osgStupeflix;

WindowCaptureCallback::ContextData::ContextData(osg::GraphicsContext* gc, ScreenCapture::Mode mode, GLenum readBuffer, const std::string& name, unsigned int startFrame):
    _gc(gc),
    _mode(mode),
    _readBuffer(readBuffer),
    _fileName(name),
    _pixelFormat(GL_BGRA),
    _type(GL_UNSIGNED_BYTE),
    _width(0),
    _height(0),
    _currentImageIndex(0),
    _currentPboIndex(0),
    _reportTimingFrequency(100),
    _numTimeValuesRecorded(0),
    _timeForReadPixels(0.0),
    _timeForFullCopy(0.0),
    _timeForMemCpy(0.0),
    _frameWritten(startFrame),
    _frameRead(startFrame)
{
    _previousFrameTick = osg::Timer::instance()->tick();

    if (gc->getTraits())
    {
        if (gc->getTraits()->alpha)
        {
            //osg::notify(osg::NOTICE)<<"Select GL_BGRA read back format"<<std::endl;
            //_pixelFormat = GL_BGRA;
            osg::notify(osg::NOTICE)<<"Select GL_RGBA read back format"<<std::endl;
            _pixelFormat = GL_RGBA;
        }
        else 
        {
            //osg::notify(osg::NOTICE)<<"Select GL_BGR read back format"<<std::endl;
            //_pixelFormat = GL_BGR; 
            osg::notify(osg::NOTICE)<<"Select GL_RGB read back format"<<std::endl;
            _pixelFormat = GL_RGB;
        }
    }
            
    getSize(gc, _width, _height);
                
    osg::notify(osg::NOTICE) << "Window size "<<_width<<", "<<_height<<std::endl;
            
    // single buffered image
    _imageBuffer.push_back(FrameIndexImage(~0x0, new osg::Image));
                
    // double buffer PBO.
    switch(_mode)
    {
    case(ScreenCapture::READ_PIXELS):
        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with out PixelBufferObject."<<std::endl;
        break;
    case(ScreenCapture::SINGLE_PBO): 
        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with a single PixelBufferObject."<<std::endl;
        _pboBuffer.push_back(0); 
        break;
    case(ScreenCapture::DOUBLE_PBO):
        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with a double buffer PixelBufferObject."<<std::endl;
        _pboBuffer.push_back(0); 
        _pboBuffer.push_back(0); 
        break;
    case(ScreenCapture::TRIPLE_PBO): 
        osg::notify(osg::NOTICE)<<"Reading window usig glReadPixels, with a triple buffer PixelBufferObject."<<std::endl;
        _pboBuffer.push_back(0); 
        _pboBuffer.push_back(0); 
        _pboBuffer.push_back(0); 
        break;
    default:
        break;                                
    }
}

void WindowCaptureCallback::ContextData::getSize(osg::GraphicsContext* gc, int& width, int& height)
{
    if (gc->getTraits())
    {
        width = gc->getTraits()->width;
        height = gc->getTraits()->height;
    }
}

std::string WindowCaptureCallback::ContextData::getImageData(unsigned int requestedFrame)
{
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_bufferImage);

        if (_pboBuffer.size() > 0) {
            if (_frameWritten  == _frameRead) {
                _hasImageToRead.wait(&_bufferImage);
            }
        }

        if (_frameRead != requestedFrame) {
            osg::notify(osg::WARN) << "frame read " << _frameRead << " requested frame " << requestedFrame << std::endl;
        }
        assert(_frameRead == requestedFrame);
        osg::Image* image = _imageBuffer[0].second.get();
        std::string str((char*)image->data(), image->getImageSizeInBytes());
        _frameRead++;

        if (_pboBuffer.size() > 0) {
            _hasNewImageToWrite.signal();
        }

        return str;
    }
    return std::string();
}

void WindowCaptureCallback::ContextData::read()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_bufferImage);
    unsigned int indexImage = _currentImageIndex;

    osg::GLBufferObject::Extensions* ext = osg::GLBufferObject::getExtensions(_gc->getState()->getContextID(),true);

    if (ext->isPBOSupported() && !_pboBuffer.empty()) {
        if (_pboBuffer.size()==1) {
            if (_frameWritten >0 && _frameWritten != _frameRead) {
                _hasNewImageToWrite.wait(&_bufferImage);
            }
            singlePBO(ext);
            _frameWritten++;
            _hasImageToRead.signal();
        } else {
            if (_frameWritten >0 && _frameWritten != _frameRead ) {
                _hasNewImageToWrite.wait(&_bufferImage);
            }
            if (multiPBO(ext)) {
                _frameWritten++;
                _hasImageToRead.signal();
            }
        }
    } else {
        readPixels();
        _frameWritten++;
    }
    _imageBuffer[indexImage].second->flipVertical();
}


void WindowCaptureCallback::ContextData::updateTimings(osg::Timer_t tick_start,
                                                       osg::Timer_t tick_afterReadPixels,
                                                       osg::Timer_t tick_afterMemCpy,
                                                       unsigned int dataSize)
{
    if (!_reportTimingFrequency) return;

    double timeForReadPixels = osg::Timer::instance()->delta_s(tick_start, tick_afterReadPixels);
    double timeForFullCopy = osg::Timer::instance()->delta_s(tick_start, tick_afterMemCpy);
    double timeForMemCpy = osg::Timer::instance()->delta_s(tick_afterReadPixels, tick_afterMemCpy);

    _timeForReadPixels += timeForReadPixels;
    _timeForFullCopy += timeForFullCopy;
    _timeForMemCpy += timeForMemCpy;
    
    ++_numTimeValuesRecorded;
    
    if (_numTimeValuesRecorded==_reportTimingFrequency)
    {
        timeForReadPixels = _timeForReadPixels/double(_numTimeValuesRecorded);
        timeForFullCopy = _timeForFullCopy/double(_numTimeValuesRecorded);
        timeForMemCpy = _timeForMemCpy/double(_numTimeValuesRecorded);
        
        double averageFrameTime =  osg::Timer::instance()->delta_s(_previousFrameTick, tick_afterMemCpy)/double(_numTimeValuesRecorded);
        double fps = 1.0/averageFrameTime;    
        _previousFrameTick = tick_afterMemCpy;

        _timeForReadPixels = 0.0;
        _timeForFullCopy = 0.0;
        _timeForMemCpy = 0.0;

        _numTimeValuesRecorded = 0;

        double numMPixels = double(_width * _height) / 1000000.0;
        double numMb = double(dataSize) / (1024*1024);

        int prec = osg::notify(osg::NOTICE).precision(5);

        if (timeForMemCpy==0.0)
        {
            osg::notify(osg::NOTICE)<<"fps = "<<fps<<", full frame copy = "<<timeForFullCopy*1000.0f<<"ms rate = "<<numMPixels / timeForFullCopy<<" Mpixel/sec, copy speed = "<<numMb / timeForFullCopy<<" Mb/sec"<<std::endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"fps = "<<fps<<", full frame copy = "<<timeForFullCopy*1000.0f<<"ms rate = "<<numMPixels / timeForFullCopy<<" Mpixel/sec, "<<numMb / timeForFullCopy<< " Mb/sec "<<
                                      "time for memcpy = "<<timeForMemCpy*1000.0<<"ms  memcpy speed = "<<numMb / timeForMemCpy<<" Mb/sec"<<std::endl;
        }
        osg::notify(osg::NOTICE).precision(prec);

    }

}

void WindowCaptureCallback::ContextData::readPixels()
{
    // std::cout<<"readPixels("<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;

    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
    unsigned int nextPboIndex = _pboBuffer.empty() ? 0 : (_currentPboIndex+1)%_pboBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        osg::notify(osg::NOTICE)<<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    osg::Image* image = _imageBuffer[_currentImageIndex].second.get();

    osg::Timer_t tick_start = osg::Timer::instance()->tick();

#if 1
    image->readPixels(0,0,_width,_height,
                      _pixelFormat,_type);
#endif

    osg::Timer_t tick_afterReadPixels = osg::Timer::instance()->tick();

    updateTimings(tick_start, tick_afterReadPixels, tick_afterReadPixels, image->getTotalSizeInBytes());

    if (!_fileName.empty())
    {
        // osgDB::writeImageFile(*image, _fileName);
    }

    _imageBuffer[_currentImageIndex].first = _frameWritten;
    _currentImageIndex = nextImageIndex;
    _currentPboIndex = nextPboIndex;
}

void WindowCaptureCallback::ContextData::singlePBO(osg::GLBufferObject::Extensions* ext)
{
    // std::cout<<"singelPBO(  "<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;

    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        osg::notify(osg::NOTICE) <<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    GLuint& pbo = _pboBuffer[0];
    
    osg::Image* image = _imageBuffer[_currentImageIndex].second.get();
    if (image->s() != _width || 
        image->t() != _height)
    {
        osg::notify(osg::NOTICE)<<"Allocating image "<<std::endl;
        image->allocateImage(_width, _height, 1, _pixelFormat, _type);
        
        if (pbo!=0)
        {
            osg::notify(osg::NOTICE)<<"deleting pbo "<<pbo<<std::endl;
            ext->glDeleteBuffers (1, &pbo);
            pbo = 0;
        }
    }
    
    
    if (pbo==0)
    {
        ext->glGenBuffers(1, &pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        osg::notify(osg::NOTICE)<<"Generating pbo "<<pbo<<std::endl;
    }
    else
    {
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo);
    }

    osg::Timer_t tick_start = osg::Timer::instance()->tick();

#if 1
    glReadPixels(0, 0, _width, _height, _pixelFormat, _type, 0);
#endif

    osg::Timer_t tick_afterReadPixels = osg::Timer::instance()->tick();

    GLubyte* src = (GLubyte*)ext->glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB,
                                              GL_READ_ONLY_ARB);
    if(src)
    {
        memcpy(image->data(), src, image->getTotalSizeInBytes());
        ext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    }

    ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    osg::Timer_t tick_afterMemCpy = osg::Timer::instance()->tick();

    updateTimings(tick_start, tick_afterReadPixels, tick_afterMemCpy, image->getTotalSizeInBytes());

    if (!_fileName.empty())
    {
        // osgDB::writeImageFile(*image, _fileName);
    }


    _imageBuffer[_currentImageIndex].first = _frameWritten;
    _currentImageIndex = nextImageIndex;
}

bool WindowCaptureCallback::ContextData::multiPBO(osg::GLBufferObject::Extensions* ext)
{
    // std::cout<<"multiPBO(  "<<_fileName<<" image "<<_currentImageIndex<<" "<<_currentPboIndex<<std::endl;
    unsigned int nextImageIndex = (_currentImageIndex+1)%_imageBuffer.size();
    unsigned int nextPboIndex = (_currentPboIndex+1)%_pboBuffer.size();

    int width=0, height=0;
    getSize(_gc, width, height);
    if (width!=_width || _height!=height)
    {
        osg::notify(osg::NOTICE) <<"   Window resized "<<width<<", "<<height<<std::endl;
        _width = width;
        _height = height;
    }

    GLuint& copy_pbo = _pboBuffer[_currentPboIndex];
    GLuint& read_pbo = _pboBuffer[nextPboIndex];
    
    osg::Image* image = _imageBuffer[_currentImageIndex].second.get();
    if (image->s() != _width ||
        image->t() != _height)
    {
        osg::notify(osg::NOTICE)<<"Allocating image "<<std::endl;
        image->allocateImage(_width, _height, 1, _pixelFormat, _type);
        
        if (read_pbo!=0)
        {
            osg::notify(osg::NOTICE)<<"deleting pbo "<<read_pbo<<std::endl;
            ext->glDeleteBuffers (1, &read_pbo);
            read_pbo = 0;
        }

        if (copy_pbo!=0)
        {
            osg::notify(osg::NOTICE)<<"deleting pbo "<<copy_pbo<<std::endl;
            ext->glDeleteBuffers (1, &copy_pbo);
            copy_pbo = 0;
        }
    }
    
    
    bool doCopy = copy_pbo!=0;
    if (copy_pbo==0)
    {
        ext->glGenBuffers(1, &copy_pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, copy_pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        osg::notify(osg::NOTICE)<<"Generating pbo "<<copy_pbo<<std::endl;
    }

    if (read_pbo==0)
    {
        ext->glGenBuffers(1, &read_pbo);
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, read_pbo);
        ext->glBufferData(GL_PIXEL_PACK_BUFFER_ARB, image->getTotalSizeInBytes(), 0, GL_STREAM_READ);

        osg::notify(osg::NOTICE)<<"Generating pbo "<<read_pbo<<std::endl;
    }
    else
    {
        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, read_pbo);
    }

    osg::Timer_t tick_start = osg::Timer::instance()->tick();

#if 1
    glReadPixels(0, 0, _width, _height, _pixelFormat, _type, 0);
#endif

    osg::Timer_t tick_afterReadPixels = osg::Timer::instance()->tick();

    if (doCopy)
    {

        ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, copy_pbo);

        GLubyte* src = (GLubyte*)ext->glMapBuffer(GL_PIXEL_PACK_BUFFER_ARB,
                                                  GL_READ_ONLY_ARB);
        if(src)
        {
            memcpy(image->data(), src, image->getTotalSizeInBytes());
            ext->glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
        }

        if (!_fileName.empty())
        {
            // osgDB::writeImageFile(*image, _fileName);
        }
    }
    
    ext->glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

    osg::Timer_t tick_afterMemCpy = osg::Timer::instance()->tick();
    
    updateTimings(tick_start, tick_afterReadPixels, tick_afterMemCpy, image->getTotalSizeInBytes());

    _imageBuffer[_currentImageIndex].first = _frameWritten;
    _currentImageIndex = nextImageIndex;
    _currentPboIndex = nextPboIndex;

    if (!doCopy) // means first frame
        return false;
    return true;
}


WindowCaptureCallback::WindowCaptureCallback(ScreenCapture::Mode mode, ScreenCapture::FramePosition position, GLenum readBuffer, unsigned int startFrame):
    _mode(mode),
    _position(position),
    _readBuffer(readBuffer),
    _contextIsReady(0),
    _frameStart(startFrame)
{
}

WindowCaptureCallback::ContextData* WindowCaptureCallback::createContextData(osg::GraphicsContext* gc) const
{
    std::stringstream filename;
    filename << "test_"<<_contextDataMap.size()<<".jpg";
    return new ContextData(gc, _mode, _readBuffer, filename.str(), _frameStart);
}
WindowCaptureCallback::ContextData* WindowCaptureCallback::getContextData(osg::GraphicsContext* gc) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

    osg::ref_ptr<ContextData>& data = _contextDataMap[gc];
    if (!data) data = createContextData(gc);
            
    if (_contextIsReady == 0) {
        _contextIsReady++;
        _waitContextIsReady.signal();
    }
    return data.get();
}

void WindowCaptureCallback::operator () (osg::RenderInfo& renderInfo) const
{
#ifndef GL_ES_VERSION_2_0
    glReadBuffer(_readBuffer);
#endif 
    osg::GraphicsContext* gc = renderInfo.getState()->getGraphicsContext();
    osg::ref_ptr<ContextData> cd = getContextData(gc);
    cd->read();
}

std::string WindowCaptureCallback::getImageData(unsigned int requestedFrame)
{
    if (!_contextIsReady) {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        if (!_contextIsReady)
            _waitContextIsReady.wait(&_mutex);
    }

    if (_contextDataMap.size() > 1) {
        osg::notify(osg::WARN) << "Number of Graphic Context " << _contextDataMap.size() << std::endl;
    }

    for (ContextDataMap::iterator it = _contextDataMap.begin(); it != _contextDataMap.end(); ++it) {
        osg::ref_ptr<ContextData> data = it->second.get();
        if (data.valid())
            return data->getImageData(requestedFrame);
    }
    return std::string();
}
