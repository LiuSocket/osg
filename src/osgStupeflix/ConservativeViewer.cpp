#include <osgStupeflix/ConservativeViewer>
#include <osgStupeflix/MarkActiveNodeVisitor>
#include <osgStupeflix/RootUpdateCallback>
#include <osgStupeflix/ImageCleanVisitor>
// #include <osgDB/Registry>

using namespace osgStupeflix;

ConservativeViewer::ConservativeViewer():
  osgViewer::Viewer()
{
    // osg::DisplaySettings::instance()->setMaxTexturePoolSize(100);
    // osg::DisplaySettings::instance()->setMaxBufferObjectPoolSize(100);
    // // IMAGE caching
    // osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options();
    // options->setObjectCacheHint(osgDB::ReaderWriter::ReaderWriter::Options::CACHE_NONE);
    // osgDB::Registry::instance()->setOptions(options);
}

void ConservativeViewer::updateTimeGroups(double simulationTime) {
    osg::Node* node = getSceneData();
    if (node == NULL) {
        return;
    }

    advance(simulationTime);

    // Reset All Nodes
    osgStupeflix::ResetAllNodeVisitor reset;
    node->accept(reset);
    
    // active current node
    osgStupeflix::MarkActiveNodeVisitor active;
    active.setCurrentTime(simulationTime);
    node->accept(active);
    
    osgStupeflix::ImageCleanVisitor clean;
    clean.setCurrentTime(simulationTime);
    node->accept(clean);
    clean.cleanupFinish();
}

bool ConservativeViewer::readyCheck(double simulationTime) {
    CheckActiveNodeVisitor check;
    check.setCurrentTime(simulationTime);

    osg::Node* sceneData = getSceneData();

    if (sceneData != NULL) {
        sceneData->accept(check);
    } else {
        return false;
    }

    std::string message;


    _fileDidNotExist = check.fileDidNotExist();
    _imageWasNotLoaded = check.imageWasNotLoaded();


    if (_fileDidNotExist || _imageWasNotLoaded) {
        message = "not loaded";
    } else {
        message = "loaded";
    }

    osg::notify(osg::INFO) << "osgStupeflix: ConservativeViewer " << message << " time = " << simulationTime << " fileDidNotExist " << _fileDidNotExist << " imageWasNotLoaded " << _imageWasNotLoaded << std::endl;

    return true;
}


bool ConservativeViewer::frameAtTime(double simulationTime)
{
    if (_done) return true;

    // OSG_NOTICE<<std::endl<<"CompositeViewer::frame()"<<std::endl<<std::endl;

    if (_firstFrame)
    {
        viewerInit();

        if (!isRealized())
        {
            realize();
        }

        _firstFrame = false;
    }

    updateTimeGroups(simulationTime);

    if (! readyCheck(simulationTime)) {
        return false;
    }

    if (_fileDidNotExist) {
        return false;
    }

    eventTraversal();
    updateTraversal();

    if (_imageWasNotLoaded) {
        return false;
    }

    renderingTraversals();
    return true;
}

