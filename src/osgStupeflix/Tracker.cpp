#include <iostream>
#include <osgStupeflix/Tracker>

using namespace osgStupeflix;

osgStupeflix::Tracker::Tracker(osg::Vec2 const &center, osg::Vec2 const &size,
                 double startTime, double endTime)
  :  _startTime(startTime),
     _endTime(endTime),
     _origCenter(center),
     _origSize(size),
     _node(NULL),
     _updateMatrix(osg::Matrix::identity()),
     _updateVector(1.f, 1.f),
     _clip(0.f, 0.f, 1.f, 1.f),
     _slowArea(0.1f, 0.1f, 0.9f, 0.9f),
     _slowPower(1.0),
     _continious(true),
     _stop(false),
     _prevSpeedCoef(1.0),
     _prevFrame(-1),
     _bufferPageSize(osgStupeflix::Tracker::DEFAULT_BUFFER_PAGE_SIZE),
     _currentIndex(0),
     _maxIndex(0),
     _saveBuffer(_bufferPageSize, center),
     _prev_center(center),
     _prev_translate(0, 0),
     _centerUniform(NULL),
     _sizeUniform(NULL)
{
  _saveBuffer[_currentIndex] = center;
}

osgStupeflix::Tracker::~Tracker() {}

bool osgStupeflix::Tracker::_isAlreadyMatched(int currentFrame) {
    if (_maxIndex >= currentFrame)
        return true;
    return false;
}

bool  osgStupeflix::Tracker::match(const unsigned char *frameData, unsigned int width, unsigned int height, double time) {
    int currentFrame = time * (double)osgStupeflix::Tracker::DEFAULT_FRAMERATE + 1;

    osg::Vec2 new_center =
        _isAlreadyMatched(currentFrame) ?
            _seek(currentFrame) :
            doMatching(frameData, width, height);

    if (_prevFrame > currentFrame)
      _prevSpeedCoef = 1.0f;
    // Smoothness between previous center and new center
    osg::Vec2 smoothed_center((new_center.x() * (1.0 - _smoothness) + _prev_center.x() * (1.0 + _smoothness)) / 2.0,
                              (new_center.y() * (1.0 - _smoothness) + _prev_center.y() * (1.0 + _smoothness)) / 2.0);

    // Translate the tracker
    this->_setTranslation(smoothed_center, (float)width / (float)height);

    // Save the center
    _prev_center = smoothed_center;

    _saveResult(currentFrame);
    _prevFrame = currentFrame;
    return true;
}

osg::Vec2 const &osgStupeflix::Tracker::_seek(int currentFrame) {
    _prev_center = _saveBuffer[currentFrame];
    return _saveBuffer[currentFrame];
}

void osgStupeflix::Tracker::_saveResult(int currentFrame) {
    if (currentFrame >= _saveBuffer.size())
        _saveBuffer.resize(_saveBuffer.size() + osgStupeflix::Tracker::DEFAULT_BUFFER_PAGE_SIZE, osg::Vec2(-1, -1));
    _saveBuffer[currentFrame] = _prev_center;
    if (currentFrame > _maxIndex)
        _maxIndex = currentFrame;
    _currentIndex = currentFrame;
}

osg::MatrixTransform *osgStupeflix::Tracker::addNode(osg::Node *node) {
    _node = new osg::MatrixTransform();
    _node->addChild(node);
    return _node;
}

void  osgStupeflix::Tracker::setSmoothness(float value) {
    _smoothness = value * 2.0 - 1.0;
}

void  osgStupeflix::Tracker::setUpdateMatrix(osg::Matrix const &updateMatrix) {
    _updateMatrix = updateMatrix;
}

void  osgStupeflix::Tracker::setUpdateVector(osg::Vec2 const &vector) {
    _updateVector = vector;
}

bool  osgStupeflix::Tracker::clip(float x1, float y1, float x2, float y2) {
    if (_origCenter.x() - _origSize.x() / 2.0 < x1 || _origCenter.x() + _origSize.x() / 2.0 > x2 ||
        _origCenter.y() - _origSize.y() / 2.0 < y1 || _origCenter.y() + _origSize.y()  / 2.0 > y2)
      return false;
    _clip.x1 = x1;
    _clip.y1 = y1;
    _clip.x2 = x2;
    _clip.y2 = y2;
    return true;
}

void  osgStupeflix::Tracker::slowArea(float x1, float y1, float x2, float y2) {
    _slowArea.x1 = x1;
    _slowArea.y1 = y1;
    _slowArea.x2 = x2;
    _slowArea.y2 = y2;
}

void osgStupeflix::Tracker::slowPower(float power) {
  _slowPower = power;
}

void osgStupeflix::Tracker::continious(bool continious) {
  _continious = continious;
}

void  osgStupeflix::Tracker::setCenterUniform(osg::Uniform *center) {
    _centerUniform = center;
    _centerUniform->set(_origCenter + _prev_translate);
}

void  osgStupeflix::Tracker::setSizeUniform(osg::Uniform *size) {
    _sizeUniform = size;
    _sizeUniform->set(_origSize);
}

float osgStupeflix::Tracker::_rangeValue(float value, float min, float max) {
  float tmp = MIN(MAX((value - min) / (max - min), 0.0), 1.0);
  tmp = 1.0 - tmp;
  return 1.0 - pow(tmp, _slowPower);
}

float osgStupeflix::Tracker::_getSpeedCoef(osg::Vec2 const &pos) {
  float coefLeft = _rangeValue(pos.x() - _origSize.x() / 2.0, _clip.x1, _slowArea.x1);
  float coefRight = _rangeValue(pos.x() + _origSize.x() / 2.0, _slowArea.x2, _clip.x2);
  coefRight  = coefRight - 1.0 * -1.0;
  float coefTop = _rangeValue(pos.y() - _origSize.y() / 2.0, _clip.y1, _slowArea.y1);
  float coefBottom = _rangeValue(pos.y() + _origSize.y() / 2.0, _slowArea.y2, _clip.y2);
  coefBottom  = coefBottom - 1.0 * -1.0;
  return MIN(coefRight, coefLeft) * MIN(coefBottom, coefTop);
}

void osgStupeflix::Tracker::_setTranslation(osg::Vec2 const &new_center, float aspect_ratio)
{
    // Difference between the original center and the new center
    osg::Vec2 translate((new_center.x() - this->_origCenter.x()) * _updateVector.x(),
                        (new_center.y() - this->_origCenter.y()) * _updateVector.y());


    float speedCoef = (_getSpeedCoef(new_center) + _getSpeedCoef(_prev_center)) / 2.0;
    if (speedCoef <= 0.02 || (_prevSpeedCoef <= 0.0 && !_continious))
      speedCoef = 0.0;
    osg::Vec2 prev = _prev_translate;
    translate[0] = prev[0] + (translate[0] - prev[0]) * speedCoef;
    translate[1] = prev[1] + (translate[1] - prev[1]) * speedCoef;
    _prev_translate = translate;
    _prevSpeedCoef = speedCoef;

    // Create a translation matrix
    osg::Matrix mat = osg::Matrix::translate(translate[0], translate[1], 0);
    // Apply the update matrix
    mat *= _updateMatrix;

    if (_node) {
        // Translate the node with the tralsation matrix
        osg::Matrix center = _node->getMatrix();
        center.makeTranslate(mat.getTrans());
        _node->setMatrix(center);
    }
    if (_centerUniform)
      _centerUniform->set(translate);
    if (_sizeUniform)
      _sizeUniform->set(translate);
}

void osgStupeflix::Tracker::_setScaling(osg::Vec2 const &new_size)
{
    // const float size_x = new_size.x() / this->_origSize.x(),
    //             size_z = new_size.y() / this->_origSize.y();

    // osg::Vec2 centerPos;
    // this->_center->get(centerPos);

    // osg::Matrix translationFrom = osg::Matrix::translate(-centerPos.x(), 0.0f, centerPos.y()-1);
    // osg::Matrix translationTo   = osg::Matrix::translate( centerPos.x(), 0.0f, 1-centerPos.y());
    // osg::Matrix scale = osg::Matrix::scale(size_x, 1.f, size_z);

    // this->_sizeTrf->setMatrix(translationFrom*scale*translationTo);
    // this->_size->set(new_size);
}

