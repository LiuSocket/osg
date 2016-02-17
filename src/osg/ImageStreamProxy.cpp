#include <iostream>
#include <osg/ImageStreamProxy>

using namespace osg;

ImageStreamProxy::ImageStreamProxy():
    ImageStream(),
    clients()
{
}

ImageStreamProxy::ImageStreamProxy(const ImageStreamProxy& image, const CopyOp& copyop):
    ImageStream(),
    clients(image.clients)
{
}

void ImageStreamProxy::registerClient(osg::ImageProxyClient* client)
{
    this->clients.push_back(client);
}

void ImageStreamProxy::updateClients(const unsigned char *data,
                                     unsigned int width, unsigned int height,
                                     double time)
{
    for (unsigned i = 0; i < this->clients.size(); i++)
        this->clients[i]->newFrame(data, width, height, time);
}
