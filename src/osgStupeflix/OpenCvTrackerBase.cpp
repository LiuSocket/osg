#if !TRACKING_NOT_SUPPORTED

#include <iostream>
#include <sstream>
#include <osgStupeflix/OpenCvTrackerBase>

OpenCvTrackerBase::OpenCvTrackerBase(cv::Vec2 const &initial_center, cv::Vec2 const &tracker_size)
: _tplSave(false),
  _templateExposureCoeff(1.0),
  _prev_move(0, 0),
  _prev_center(initial_center),
  _size(tracker_size)
{}

OpenCvTrackerBase::OpenCvTrackerBase(cv::Vec2 const &initial_center, cv::Vec2 const &tracker_size, cv::Vec2 const &video_size)
: _tplSave(false),
  _templateExposureCoeff(1.0),
  _prev_move(0, 0),
  _prev_center(initial_center.x / video_size.x, initial_center.y / video_size.y),
  _size(tracker_size.x / video_size.x, tracker_size.y / video_size.y)
{}

void OpenCvTrackerBase::_updateTemplate(unsigned int tplw, unsigned int tplh, unsigned int tplx, unsigned int tply,
                                        const unsigned char *data, unsigned int width, unsigned int height)
{
    // // Crop the template if it leave the image
    // tplw = CLIP(tplw, 0, width  - tplx);
    // tplh = CLIP(tplh, 0, height - tply);

    const unsigned int linesize = width; // XXX: probably incorrect sometimes (presence of padding)

    // Copy the area of the template
    this->_tpl = cv::Mat(tplh, tplw, CV_8U,
                         (void*)(data + tply*linesize + tplx),
                         linesize).clone();

    const int searchw = MIN(this->_tpl.cols * 3, width),
              searchh = MIN(this->_tpl.rows * 3, height),
              searchx = CLIP((int)tplx + (int)tplw / 2 - searchw / 2, 0, width - searchw),
              searchy = CLIP((int)tply + (int)tplh / 2 - searchh / 2, 0, height - searchh);

    _templateExposureCoeff = 0;
    for (unsigned int x = searchx; x < searchx + searchw; ++x) {
      for (unsigned int y = searchy; y < searchy + searchh; ++y) {
        _templateExposureCoeff += data[x + y * width] * data[x + y * width];
      }
    }
    _templateExposureCoeff = sqrt(_templateExposureCoeff);
}


bool OpenCvTrackerBase::doMatching(const unsigned char *frameData, unsigned int width,
                                   unsigned int height)
{
    // Create an OpenCV wrapper for the input matrix
    cv::Mat frame(height, width, CV_8UC4, (void*)frameData);

    // Downsample that if necessary
    float scale = MIN(MIN(MAX_SIZE/frame.cols,MAX_SIZE/frame.rows),1);
    if (scale < 1) {
        cv::Size newSize(round(scale*frame.cols),round(scale*frame.rows));
        cv::resize(frame,frame,newSize,0,0);
    }

    // Convert RGBA -> Gray
    cv::cvtColor(frame, frame, cv::COLOR_RGBA2GRAY);

    // Get the frame dimensions
    width = frame.cols;
    height = frame.rows;

    // Get the template dimensions
    const unsigned int tplw = _size.x * width,
                       tplh = _size.y * height;

    const unsigned int center_wshift = (tplw + 1) / 2,
                       center_hshift = (tplh + 1) / 2;

    // If there is no template saved, init the template
    if (!_tplSave) {
        const unsigned int tplx = CLIP(_prev_center.x * width  - center_wshift, 0, width  - 1),
                           tply = CLIP(_prev_center.y * height - center_hshift, 0, height - 1);
        this->_updateTemplate(tplw, tplh, tplx, tply, frame.data, width, height);
        _tplSave = true;
    }

    // Check errors
    if ((unsigned)this->_tpl.cols > width || (unsigned)this->_tpl.rows > height)
         return false;

    // Get the search area dimensions & position
    const unsigned int searchw = MIN(this->_tpl.cols * 3, width),
                       searchh = MIN(this->_tpl.rows * 3, height),
                       searchx = CLIP((_prev_center.x + _prev_move.x) * width - searchw / 2, 0, width - searchw),
                       searchy = CLIP((_prev_center.y + _prev_move.y) * height - searchh / 2, 0, height - searchh);

    cv::Mat res;
    // Create the cv::Mat for the search area
    cv::Mat img = cv::Mat(searchh, searchw, CV_8UC1,
                  (unsigned char *)frame.data + searchy*width + searchx , width).clone();

    // Check if the exposition changed
    double exposureCoeff = _getExposureCoeff(img, cv::Rect(0, 0, searchw, searchh));

    // Adjust the template exposure
    cv::Mat newTpl = _adjustTemplateExposure(exposureCoeff);

    // Try to match the template
    cv::matchTemplate(img, newTpl, res, cv::TM_SQDIFF);

    double minVal;
    cv::Point minLoc;
    // Get the minVal and minValLocation to determine where the template is match
    cv::minMaxLoc(res, &minVal, NULL, &minLoc, NULL);

    // Calculate the new center of the tracker
    double new_center_x = (minLoc.x + center_wshift + searchx) / (double)width,
           new_center_y = (minLoc.y + center_hshift + searchy) / (double)height;

    // Save the previous movement
    cv::Vec2 move = cv::Vec2(new_center_x - _prev_center.x, new_center_y - _prev_center.y);

    double prev_move_len = sqrt(_prev_move.x * _prev_move.x + _prev_move.y * _prev_move.y);
    double move_len = sqrt(move.x * move.x + move.y * move.y);

    double diff = sqrt((move.x - _prev_move.x) * (move.x - _prev_move.x) +
                       (move.y - _prev_move.y) * (move.y - _prev_move.y)) / prev_move_len;

    // Check if the tracker jumped
    if ((prev_move_len >= 0.008 || move_len >= 0.03) && diff >= 3.5) {
      // If it jumped :
      // set the new center with the previous movement
      new_center_x = _prev_center.x + _prev_move.x;
      new_center_y = _prev_center.y + _prev_move.y;

      // Update the template
      const unsigned int tplx = CLIP(_prev_center.x * width  - center_wshift, 0, width  - 1),
                         tply = CLIP(_prev_center.y * height - center_hshift, 0, height - 1);
      // Check if the template is out of the screen
      this->_updateTemplate(tplw, tplh, tplx, tply, frame.data, width, height);
    }
    else {
      // save the movement
      _prev_move = move;
    }
    // Save the center
    _prev_center = cv::Vec2(new_center_x, new_center_y);
    return true;
}

double  OpenCvTrackerBase::_getExposureCoeff(cv::Mat const &mat, cv::Rect const &box) {
  double coeff = 0;
  for (unsigned int x = 0; x < box.width; ++x)
    for (unsigned int y = 0; y < box.height; ++y)
      coeff += mat.data[x + box.x + (y + box.y) * mat.cols] * mat.data[x + box.x + (y + box.y) * mat.cols];
  coeff = sqrt(coeff);
  return coeff / _templateExposureCoeff;
}

cv::Mat  OpenCvTrackerBase::_adjustTemplateExposure(double coeff) {
    cv::Mat newTpl = cv::Mat(this->_tpl.rows, this->_tpl.cols, CV_8U);
    unsigned char *newData = newTpl.data;
    unsigned char *prevData = this->_tpl.data;
    for (unsigned int i = 0; i < this->_tpl.cols * this->_tpl.rows; ++i) {
      if (coeff > 1. && prevData[i] * coeff > 0xFF)
        newData[i] = 0xFF;
      else
        newData[i] = prevData[i] * coeff;
    }
    return newTpl;
}

OpenCvTrackerBase::~OpenCvTrackerBase()
{}

cv::Vec2 const &OpenCvTrackerBase::size() const {
  return _size;
}

cv::Vec2 const &OpenCvTrackerBase::movement() const {
  return _prev_move;
}

cv::Vec2 const &OpenCvTrackerBase::center() const {
  return _prev_center;
}

void            OpenCvTrackerBase::set_center(cv::Vec2 const &position, cv::Vec2 const &video_size) {
    _prev_center.x = position.x / video_size.x;
    _prev_center.y = position.y / video_size.y;
}

void            OpenCvTrackerBase::set_center(cv::Vec2 const &position) {
    _prev_center.x = position.x;
    _prev_center.y = position.y;
}

#endif
