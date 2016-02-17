#include <iostream>
#include <stdexcept>
#include <string.h>
#include <osgStupeflix/VideoBuffer>

#ifdef ANDROID
#include <malloc.h>
#else
#include <stdlib.h>
#endif

using namespace osgStupeflix;

VideoBuffer::VideoBuffer(int w, int h, void *data, size_t bpp, ssize_t bpr, bool copy)
:   _data(data),
    _w(w),
    _h(h),
    _bpp(bpp),
    _freeWhenDone(false),
    pixelFormat(0)
{


    if (bpr > 0)
        _bpr = bpr;
    else 
        _bpr = _bpp*w;

    _size = _bpr*_h;

    if (data == NULL){
#ifdef ANDROID
        if ((_data = memalign(16, _size)) == NULL) {
#else
        if (posix_memalign(&_data, 16, _size) != 0) {
#endif
            throw std::runtime_error("Error: osgStupeflix::VideoBuffer() : Could not allocate buffer");
        }
        _freeWhenDone = true;
    }

    if (copy)
    {
        if (data == NULL){
            throw std::runtime_error("Error: osgStupeflix::VideoBuffer(): copy = true && data == NULL");
        }
#ifdef ANDROID
        if ((_data = memalign(16, _size)) == NULL) {
#else
        if (posix_memalign(&_data, 16, _size) != 0) {
#endif
            throw std::runtime_error("Error: osgStupeflix::VideoBuffer() : Could not allocate buffer");
        }
        _freeWhenDone = true;
        memcpy(_data, data, _size);
    } 
}


VideoBuffer::~VideoBuffer()
{
    if (_freeWhenDone)
        free(_data);

}
