#include <string>
#include <algorithm>
#include <stdexcept>

#include <osgStupeflix/FbxAnimation>

#include <osg/Node>

#include <osgAnimation/Animation>
#include <osgAnimation/BasicAnimationManager>

#include <osgDB/ReadFile>

namespace osgStupeflix {

  /******************************** CTOR **************************************/

  FbxAnimation::~FbxAnimation()
  {
  }

  FbxAnimation::FbxAnimation(std::string const &filename)
    : _node(osgDB::readNodeFile(filename))
  {
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    apply(*_node.get());
  }

  /******************************* public member function *********************/

  void
  FbxAnimation::apply(osg::Node &node)
  {
    if (_anim.valid())
      return;
    if (node.getUpdateCallback()) {
      _anim = dynamic_cast<osgAnimation::BasicAnimationManager*>(node.getUpdateCallback());
      loadAnimationNames();
    }
    else
      traverse(node);
  }

  void
  FbxAnimation::selectAnimation(std::string const &animationName)
  {
    _selected = getAnimation(animationName);
  }

  osg::ref_ptr<osg::Node>
  FbxAnimation::getNode() const
  {
    return _node;
  }

  std::vector<std::string> const &
  FbxAnimation::getAnimationsNames() const
  {
    return _animNameList;
  }

  double
  FbxAnimation::getDuration() const
  {
    if (!_selected.valid())
      throw FbxException("no animation selected");
    return _selected->getDuration();
  }

  void
  FbxAnimation::setDuration(double duration) const
  {
    if (!_selected.valid())
      throw FbxException("No animation selected");
    _selected->setDuration(duration);
  }

  void
  FbxAnimation::playAnimation() const
  {
    if (!_selected.valid())
      throw FbxException("No animation selected");
    _anim->playAnimation(_selected);
  }

  osgAnimation::Animation::PlayMode
  FbxAnimation::getPlayMode() const
  {
    if (!_selected.valid())
      throw FbxException("No animation selected");
    return _selected->getPlayMode();
  }

  void
  FbxAnimation::setPlayMode(osgAnimation::Animation::PlayMode mode) const
  {
    if (!_selected.valid())
      throw FbxException("No animation selected");
    _selected->setPlayMode(mode);
  }

  /********************************* private member function ******************/

  void
  FbxAnimation::loadAnimationNames() {
    using namespace osgAnimation;

    AnimationList const &l = _anim->getAnimationList();
    for (AnimationList::const_iterator it = l.begin(); it != l.end(); ++it)
      _animNameList.push_back((*it)->getName());
  }

  osgAnimation::Animation *
  FbxAnimation::getAnimation(std::string const &name) const
  {
    using namespace osgAnimation;

    AnimationList const &l = _anim->getAnimationList();
    for (AnimationList::const_iterator it = l.begin(); it != l.end(); ++it) {
      if ((*it)->getName() == name)
	return  *it;
    }
    throw FbxException("Animation not found");
    return *l.end();
  }

  /********************************** FBXexception ****************************/

  FbxException::FbxException(std::string const &error)
    : _error(error)
  {
  }

  FbxException::~FbxException() throw()
  {
  }

  char const *
  FbxException::what() const throw()
  {
    return _error.c_str();
  }
}
