/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <osg/GL>
#include <osg/TexMat>
#include <osg/Notify>
#include <osg/TextureRectangle>
#include <osg/io_utils>
#include <osgStupeflix/TexMat>
#include <assert.h>
#include <sstream>

using namespace osg;
using namespace osgStupeflix;



#define OVERRIDE_IMPLEMENTATION 1


DefaultUpdateTexMat::DefaultUpdateTexMat()
{
}

DefaultUpdateTexMat::DefaultUpdateTexMat(const DefaultUpdateTexMat& updateTexMat,const osg::CopyOp& copyop)
{
}

// Callback method called by the NodeVisitor when visiting a node.
void DefaultUpdateTexMat::operator()(osg::StateAttribute* node, osg::NodeVisitor* nv) {
    osgStupeflix::TexMat* texMat = dynamic_cast<osgStupeflix::TexMat*>(node);

    if (!texMat) {
        osg::notify(osg::INFO) << "This is not a osgStupeflix::TexMat !" << std::endl;
        return;
    }

    Uniform* texMatUniform = texMat->getUniform();

    texMatUniform->set(texMat->getMatrix());

    texMat->addToParentStateSet();
}

void osgStupeflix::TexMat::setMatrix(const Matrix& matrix) {
    osg::TexMat::setMatrix(matrix);
    // TODO : use _scaleByTextureRectangleSize
#if 0
    if(_scaleByTextureRectangleSize) {
        const osg::TextureRectangle* tex = dynamic_cast<const osg::TextureRectangle*>(state.getLastAppliedTextureAttribute(state.getActiveTextureUnit(), osg::StateAttribute::TEXTURE));
        *_tmpMatrix = _matrix * osg::Matrix::scale(osg::Vec3f(tex->getTextureWidth(),tex->getTextureHeight(),1.0));
        _texMatUniform->set(*_tmpMatrix);
    } else {
        _texMatUniform->set(_matrix);
    }
#else
    _texMatUniform->set(_matrix);
#endif
}

osgStupeflix::TexMat::~TexMat()
{
    delete _tmpMatrix;
}

void osgStupeflix::TexMat::initUniform() {
    std::stringstream st;
    st << "osg_TextureMatrix" << _index;
    std::string uniform_name =  st.str();

    _texMatUniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, uniform_name);

    _texMatUniform->set(_matrix);
    setUpdateCallback(new DefaultUpdateTexMat());
}

void osgStupeflix::TexMat::addToParentStateSet() {
    unsigned int numParents = this->getNumParents();
    unsigned int i;

    //    osg::notify(osg::INFO) << "TexMat addToParentStateSet: got " << numParents << " parents." << this->getMatrix() << std::endl;

    for(i = 0; i < numParents ; i++)
    {
        StateSet* stateset = this->getParent(i);
        Uniform* uniform = stateset->getUniform(_texMatUniform->getName());
        if (uniform == NULL)
        {
            //            osg::notify(osg::INFO) << "TexMat addToParentStateSet : got null uniform." << std::endl;
            stateset->addUniform(_texMatUniform);
        } else {
            //            osg::notify(osg::INFO) << "TexMat addToParentStateSet : got non null uniform." << std::endl;
        }
    }
}

void osgStupeflix::TexMat::apply(State& state) const
{
}
