/*  -*-c++-*-
 *  Copyright (C) 2012 Stupeflix
 *
 * Authors:
 *        François Lagunas <francois.lagunas@gmail.com>
 */

#include <osgStupeflix/RenderRange>
#include <osgUtil/CullVisitor>
#include <assert.h>

using namespace osgStupeflix;
#define RENDERRANGE_DEBUG 0

osgStupeflix::RenderRange::RenderRange(const RenderRange& b, const osg::CopyOp& copyop) : 
  RenderOnce(b, copyop),
  _lastAccessRangeIndex(RR_INVALID_RANGE_INDEX),
  _lastDrawRangeIndex(RR_INVALID_RANGE_INDEX),
  _lastDrawTime(RR_INVALID_LAST_DRAW_TIME)
{}

osgStupeflix::RenderRange::RenderRange() :
  RenderOnce(),
  _lastAccessRangeIndex(RR_INVALID_RANGE_INDEX),
  _lastDrawRangeIndex(RR_INVALID_RANGE_INDEX),
  _lastDrawTime(RR_INVALID_LAST_DRAW_TIME)
{}

// Find the range for the given time, or RR_INVALID_RANGE_INDEX if not found
int osgStupeflix::RenderRange::findRange(float time) {
    int searchStart;
    if (_lastAccessRangeIndex != RR_INVALID_RANGE_INDEX && time >= _ranges[_lastAccessRangeIndex].start) {
        // linear search starting at last accessed range because it's the most common case
        searchStart = _lastAccessRangeIndex;
    } else {
        // linear search from begining
        _lastAccessRangeIndex = RR_INVALID_RANGE_INDEX;
        searchStart = 0;
    }
    // search
    for(int i = searchStart; i < _ranges.size() ; i++) {
        if (time < _ranges[i].start) {
            break;
        }
        _lastAccessRangeIndex = i;
    }
    return _lastAccessRangeIndex;
}

void osgStupeflix::RenderRange::reset() {
#if RENDERRANGE_DEBUG
    osg::notify(osg::INFO) << "RenderRange reset" << std::endl;
#endif
    _lastAccessRangeIndex = RR_INVALID_RANGE_INDEX;
    _lastDrawRangeIndex = RR_INVALID_RANGE_INDEX;
    _lastDrawTime = RR_INVALID_LAST_DRAW_TIME;
}

osg::Node::NodeMask osgStupeflix::RenderRange::nodeMask(float time) {
#if RENDERRANGE_DEBUG
    osg::notify(osg::INFO) << "RenderRange nodeMask at " << time << " : "  << std::endl;
#endif
    int rangeIndex  = findRange(time);
    if (rangeIndex == RR_INVALID_RANGE_INDEX) {
        return 0;
    }
    
    Range range = _ranges[rangeIndex];
    if (range.type == CONTINUOUS) {
        return ACTIVE | PREFETCH;
    } else {
        osg::Node::NodeMask mask;
        if (range.updateEnable && rangeIndex != (_ranges.size() - 1)) {
            mask = PREFETCH;
        } else {
            mask = 0;
        }
        if (range.type == ONCE) {
            if (rangeIndex != _lastDrawRangeIndex) {
                mask |= ACTIVE;
            }
            return mask;
        } else if (range.type == NO_RENDER) {
            return mask;
        }
    }
    assert(false);
    return 0;
}

bool osgStupeflix::RenderRange::needCleaning(float time) {
#if RENDERRANGE_DEBUG
    osg::notify(osg::INFO) << "RenderRange needCleaning at " << time << " : " << std::endl;
#endif
    int rangeIndex = findRange(time);

    if (rangeIndex == RR_INVALID_RANGE_INDEX) {
        return false;
    }
    Range range = _ranges[rangeIndex];
    if (range.type == CONTINUOUS) {
        return false;
    }

    if (rangeIndex == (_ranges.size() - 1) && rangeIndex == _lastDrawRangeIndex) {
        return true;
    } else {
        return false;
    }
}

void osgStupeflix::RenderRange::wasDrawn(float time) {
#if RENDERRANGE_DEBUG
    osg::notify(osg::INFO) << "RenderRange wasDrawn" << std::endl;
#endif
    int rangeIndex = findRange(time);
    _lastDrawRangeIndex = rangeIndex;
    _lastDrawTime = time;
}

void osgStupeflix::RenderRange::addRange(float start, RenderRange::RangeType type, bool updateEnable) {
#if RENDERRANGE_DEBUG
    osg::notify(osg::INFO) << "RenderRange addRange (start=" << start << ") in ";
    debugRanges();
    osg::notify(osg::INFO) << std::endl;
#endif

    struct Range range;
    range.start = start;
    range.type = type;

    // insert new range at right place
    int rangeIndex = findRange(start);
    if (_ranges.size() == 0) {
        _ranges.push_back(range);        
    } else if (rangeIndex == RR_INVALID_RANGE_INDEX) {
        _ranges.insert(_ranges.begin(), range);
    } else if (rangeIndex == _ranges.size() - 1) {
        _ranges.push_back(range);
    } else {
        _ranges.insert(_ranges.begin() + rangeIndex + 1, range);
    }

#if RENDERRANGE_DEBUG
    osg::notify(osg::INFO) << "   Result: ";
    debugRanges();
    osg::notify(osg::INFO) << std::endl;
#endif    
}

void osgStupeflix::RenderRange::debugRanges() {
    osg::notify(osg::INFO) << _ranges.size() << " range(s): ";
    for (int i = 0; i < _ranges.size(); i++) {
        osg::notify(osg::INFO) << "(" <<_ranges[i].start << ", ";
        if (_ranges[i].type == ONCE) {
            osg::notify(osg::INFO) << "ONCE";
        } else if (_ranges[i].type == CONTINUOUS) {
            osg::notify(osg::INFO) << "CONTINUOUS";
        } else if (_ranges[i].type == NO_RENDER) {
            osg::notify(osg::INFO) << "NO_RENDER";
        }
        osg::notify(osg::INFO) << ", ";
        osg::notify(osg::INFO) << (_ranges[i].updateEnable ? "update" : "no update");
        osg::notify(osg::INFO) << ")";
        if (i < _ranges.size() - 1) {
            osg::notify(osg::INFO) << ", ";
        }
    }
}
