/*  -*-c++-*- 
 *  Copyright (C) 2010 Stupeflix 
 *
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */


#include <osgStupeflix/DrawUpdateCallback>
#include <osg/Geometry>
#include <osg/io_utils>

using namespace osgStupeflix;



MultiDrawUpdateCallback::MultiDrawUpdateCallback() {}
void MultiDrawUpdateCallback::update(osg::NodeVisitor* nv, osg::Drawable* dw)
{
    osg::Geometry* geom = dynamic_cast<osg::Geometry*>(dw);
    if (!geom)
        return;
    for (unsigned int i = 0; i < _uvs.size(); ++i) {
        if (_originalUVS.size() <= i) {
            _originalUVS.push_back(dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(i)->clone(osg::CopyOp::DEEP_COPY_ALL)));
        }
    }

    for (unsigned int i = 0; i < _uvs.size(); ++i) {
        osg::Vec2Array* uv = _originalUVS[i].get();
        if (uv && _uvs[i].valid()) {
            _uvs[i]->update(nv, dw, uv, i);
        }
    }
}
#if 0
void MultiDrawUpdateCallback::cloneUV(osg::Geometry* target)
{
    if (!target->getTexCoordArray(0)) {
        osg::notify(osg::WARN) << "geometry " << target->getName() << " has not tex coord" << std::endl;
    }
    _originalUVs = target->getTexCoordArray(0)->clone(osg::CopyOp::DEEP_COPY);
}
#endif

DrawUpdateCallback::DrawUpdateCallback():osg::Drawable::UpdateCallback() ,_uvs(new osg::Vec2Array(4)) 
{
    _uvs->setDataVariance(osg::Object::DYNAMIC);
    _start = 0;
    _duration = 1.0;
}

osg::Vec2Array* DrawUpdateCallback::getUVArray() { return _uvs.get(); }

void DrawUpdateCallback::setSourceGeometry(osg::Geometry* geom) { _geometry = geom; }

void DrawUpdateCallback::update(osg::NodeVisitor* nv, osg::Drawable* drw)
{
    osg::Vec2Array* uv = 0;
    uv = dynamic_cast<osg::Vec2Array*>(_geometry->getTexCoordArray(0));
    update(nv, drw, uv);
}

void DrawUpdateCallback::update(osg::NodeVisitor* nv, osg::Drawable* drw, osg::Vec2Array* uv, int unit)
{
    double t = nv->getFrameStamp()->getSimulationTime();
    osg::notify(osg::INFO) << "osgStupeflix: time " << t << " " << this << " execute callback for geometry " << drw << " " << _nestedCallback.get() <<std::endl;
    if (_nestedCallback.valid()) {
        _nestedCallback->update(nv, drw, uv, unit);
    } else {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(drw);
        if (geom) {
            osg::Vec2Array* uvDest = dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit));
            osg::Vec2Array* arrayDst = uv;
            osg::Vec2 center = osg::Vec2(0,0);
            for (int i = 0; i < 4; i++) {
                (*uvDest)[i] = (*arrayDst)[i];
                center += (*arrayDst)[i];
            }
            center *= 0.25;
            uvDest->dirty();
            geom->dirtyDisplayList();
            geom->dirtyBound();

            osg::Uniform* uniform = geom->getOrCreateStateSet()->getOrCreateUniform("texCenter", osg::Uniform::FLOAT_VEC2, 1);
            uniform->set(center);
            osg::notify(osg::INFO) << "osgStupeflix: time " << t << " " << "computed texCenter " << center << std::endl;
        }
    }
}

void DrawUpdateCallback::interpolate(double t, const osg::Vec2Array& src, const osg::Vec2Array& dst, osg::Vec2Array& result)
{
    result.resize(4);
    if (t >= 1.0) {
        for (int i = 0; i < 4; i++) {
            result[i] = dst[i];
        }
        return;
    } else if (t <= 0) {
        for (int i = 0; i < 4; i++) {
            result[i] = src[i];
        }
        return;
    }

    double t1 = 1.0 - t;
    for (int i = 0; i < 4; i++) {
        result[i] = src[i]*t1 + dst[i]*t;
    }
}
