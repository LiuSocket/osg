#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgText/Text>
#include <osgText/Font>
#include <osgViewer/Viewer>
#include <osgStupeflix/Tracking>
#include <osgStupeflix/OpenCvTracker>



#include <osgGA/TrackballManipulator>



class myviewer : public osgViewer::Viewer {
public:
    virtual int run(double time = 1. / 60.) {
        if (!getCameraManipulator() && getCamera()->getAllowEventFocus())
        {
            setCameraManipulator(new osgGA::TrackballManipulator());
        }

        setReleaseContextAtEndOfFrameHint(false);

        if (!isRealized())
        {
            realize();
        }

        const char* run_frame_count_str = getenv("OSG_RUN_FRAME_COUNT");
        unsigned int runTillFrameNumber = run_frame_count_str==0 ? osg::UNINITIALIZED_FRAME_NUMBER : atoi(run_frame_count_str);

        int frame_number = 1;


        std::string line;

        while(!done() && (run_frame_count_str==0 || getViewerFrameStamp()->getFrameNumber()<runTillFrameNumber))
        {
            // std::cout << "FRAME = " << frame_number << std::endl;
            ++frame_number;
            double minFrameTime = _runMaxFrameRate>0.0 ? 1.0/_runMaxFrameRate : 0.0;
            osg::Timer_t startFrameTick = osg::Timer::instance()->tick();
            if (_runFrameScheme==ON_DEMAND)
            {
                if (checkNeedToDoFrame())
                {
                    frame(time);
                }
                else
                {
                    // we don't need to render a frame but we don't want to spin the run loop so make sure the minimum
                    // loop time is 1/100th of second, if not otherwise set, so enabling the frame microSleep below to
                    // avoid consume excessive CPU resources.
                    if (minFrameTime==0.0) minFrameTime=0.01;
                }
            }
            else
            {
                frame(time);
            }

            // work out if we need to force a sleep to hold back the frame rate
            osg::Timer_t endFrameTick = osg::Timer::instance()->tick();
            double frameTime = osg::Timer::instance()->delta_s(startFrameTick, endFrameTick);
            if (frameTime < minFrameTime) OpenThreads::Thread::microSleep(static_cast<unsigned int>(1000000.0*(minFrameTime-frameTime)));

            line.clear();

            std::getline(std::cin, line);
            if (line.empty()) {
                time += 1. / 30.;
            } else {
                double value = std::atof(line.c_str());
                time = value;
            }
        }

        return 0;
    }
};

/*
 *             grp (Group)
 *           /     \
 *    geode_video  size_trf (Tracker size Matrix Scaling)
 *        /          \
 *     video        center_trf (Tracker center Matrix Translation)
 *                   /     \
 *             text_trf  geode_tracking
 *                /            \
 *            geode_text     overlay image
 *              /
 *            text
 */
static osg::Node* getScene(const char *ifname, const char *overlay, float x, float y)
{
    osg::Image* video = osgDB::readImageFile(ifname);
    if (!video) {
        std::cerr << "osgDB::readImageFile('" << ifname << "') return NULL" << std::endl;
        return NULL;
    }
    osg::Image* overlay_img = osgDB::readImageFile(overlay);
    if (!overlay_img) {
        std::cerr << "osgDB::readImageFile('" << overlay << "') return NULL" << std::endl;
        return NULL;
    }
    osg::Geode* geode_video    = new osg::Geode;
    osg::Geode* geode_tracking = new osg::Geode;

    osg::Uniform* match   = new osg::Uniform(osg::Uniform::FLOAT, "match_confidence");
    osg::Uniform* center  = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "center");
    osg::Uniform* size    = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "size");

    geode_tracking->getOrCreateStateSet()->addUniform(match);
    geode_tracking->getOrCreateStateSet()->addUniform(center);
    geode_tracking->getOrCreateStateSet()->addUniform(size);
    geode_tracking->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON); // support alpha in overlay


    /*
     *             x
     *                   1
     *     0 + --------- . >
     *       |
     *  y    |     * tracker (center position and size in [0,1],
     *       |                source video scale)
     *       |
     *     1 .
     *       v
     */
    const int vidw  = video->s(),       vidh  = video->t();
    std::cout << "video size : " << vidw << " " << vidh << std::endl;
    const int overw = overlay_img->s(), overh = overlay_img->t();
    osg::Vec2 initial_tracker_pos  = osg::Vec2(x, y);
    osg::Vec2 initial_tracker_size = osg::Vec2(overw / ((float)vidw), overh / ((float)vidh));

    /* Quad for video */
    const float maxsz = vidw > vidh ? vidw : vidh;
    const float img_container_scale = 1.f / maxsz;
    const float svideo_w = vidw * img_container_scale;
    const float svideo_h = vidh * img_container_scale;
    osg::Vec3 corner = osg::Vec3(     0.0,  0.0,      0.0);
    osg::Vec3 width  = osg::Vec3(svideo_w,  0.0,      0.0);
    osg::Vec3 height = osg::Vec3(     0.0,  0.0, svideo_h);
    osg::Geometry* geom_video = osg::createTexturedQuadGeometry(corner, width, height, 0.0, 1.0, 1.0, 0.0);

    /* Quad for overlay image */
    const float sover_w = overw * img_container_scale;
    const float sover_h = overh * img_container_scale;
    const float pos_x = (      initial_tracker_pos.x()  - sover_w/2.) * svideo_w;
    const float pos_y = ((1. - initial_tracker_pos.y()) - sover_h/2.) * svideo_h;
    corner = osg::Vec3(  pos_x, -.01,   pos_y);
    width  = osg::Vec3(sover_w,  0.0,     0.0);
    height = osg::Vec3(    0.0,  0.0, sover_h);
    osg::Geometry* geom_over = osg::createTexturedQuadGeometry(corner, width, height, 0.0, 0.0, 1.0, 1.0);

    /* Set quad textures and wrap them into their respective geode */
    osg::Texture2D* videotex = new osg::Texture2D(video);
    osg::Texture2D* overtex  = new osg::Texture2D(overlay_img);
    videotex->setResizeNonPowerOfTwoHint(false);
    overtex ->setResizeNonPowerOfTwoHint(false);
    geom_video->getOrCreateStateSet()->setTextureAttributeAndModes(0, videotex);
    geom_over ->getOrCreateStateSet()->setTextureAttributeAndModes(0, overtex);
    geode_video->addDrawable(geom_video);
    geode_tracking->addDrawable(geom_over);

    /* Text & Text Geode */
    osgText::Font* font = osgText::readFontFile("fonts/arial.ttf");
    osg::Vec4 textColor(1.0f,.0f,.0f,1.0f);

    osgText::Text* text = new osgText::Text();
    text->setColor(textColor);
    text->setFont(font);
    text->setFontResolution(512,512);
    text->setCharacterSize(0.05);
    text->setText("");

    osg::Geode* geode_text = new osg::Geode();
    geode_text->addDrawable(text);

    /* MatrixTransform for tracker center translation */
    osg::MatrixTransform* center_trf = new osg::MatrixTransform;
    center_trf->addChild(geode_tracking);

    /* MatrixTransform for tracker scaling */
    osg::MatrixTransform* size_trf = new osg::MatrixTransform;
    size_trf->addChild(center_trf);

    /* MatrixTransform for text */
    osg::Matrix textTranslation = osg::Matrix::translate(pos_x+0.1f, pos_y+0.00, 0.1f);
    osg::Matrix textRotation = osg::Matrix::rotate(M_PI/2,1.0f, 0.f, 0.f);

    osg::MatrixTransform* text_trf = new osg::MatrixTransform();
    text_trf->setMatrix(textTranslation*textRotation);
    text_trf->addChild(geode_text);

    center_trf->addChild(text_trf);


    /* Add a tracker of the size of the image overlay */
    osg::ImageStreamProxy* isp = dynamic_cast<osg::ImageStreamProxy*>(video);

    osgStupeflix::Tracking* tracking = new osgStupeflix::Tracking();


    osg::Matrix update_matrix =
        osgStupeflix::TrackingMatrixUtils::desktop_transform((double)svideo_w / (double)svideo_h);

    osg::ref_ptr<osgStupeflix::Tracker> tracker =
        new osgStupeflix::OpenCvTracker(initial_tracker_pos,
                                        initial_tracker_size);

    osg::ref_ptr<osg::MatrixTransform> node = tracker->addNode(size_trf);
    tracker->setSmoothness(0.5f);
    tracker->setUpdateMatrix(update_matrix);
    tracker->setUpdateVector(osg::Vec2(1.0, 1.0));
    tracker->clip(0.25f, 0.25f, 0.75f, 0.75f);
    tracker->slowArea(0.4f, 0.4f, 0.6f, 0.6f);
    tracker->continious(false);

    tracking->addTracker(tracker);
    isp->registerClient(tracking);

    /* Final tree construction */
    osg::Group* grp = new osg::Group;
    grp->addChild(geode_video);
    grp->addChild(node);
    return grp;
}

int main(int ac, char **av)
{
    if (ac < 3) {
        std::cout << "Usage: " << av[0] << " <video> <image> <start_x> <start_y>" << std::endl;
        return 0;
    }
    std::cout << "Enter to continue | number to seek" << std::endl;
    myviewer viewer;

    float x = .5, y = .5;
    if (ac>=4) x = atof(av[3]);
    if (ac>=5) y = atof(av[4]);

    viewer.setSceneData(getScene(av[1], av[2],x,y));
    return viewer.run();
}
