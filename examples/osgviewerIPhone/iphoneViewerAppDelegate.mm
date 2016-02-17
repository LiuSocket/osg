//Created by Thomas Hogarth 2009

//
//This exampe shows how to render osg into an existing windw. Apple recommends apps only have one window on IPhone so this
// will be your best bet.
//

#import "iphoneViewerAppDelegate.h"
#include <osgGA/TrackballManipulator>
#include <osg/ShapeDrawable>
//inckude the iphone specific windowing stuff
#include <osgViewer/api/IOS/GraphicsWindowIOS>




#define kAccelerometerFrequency 30.0 // Hz
#define kFilteringFactor 0.1

static const char gVertexShader[] =
"varying vec4 color;                                                    \n"
"const vec3 lightPos      =vec3(0.0, 0.0, 10.0);                        \n"
"const vec4 modelColor   =vec4(0.8, 0.8, 0.8, 1.0);                     \n"
"const vec4 lightAmbient  =vec4(0.1, 0.1, 0.1, 1.0);                    \n"
"const vec4 lightDiffuse  =vec4(0.4, 0.4, 0.4, 1.0);                    \n"
"const vec4 lightSpecular =vec4(0.8, 0.8, 0.8, 1.0);                    \n"
"void DirectionalLight(in vec3 normal,                                  \n"
"                      in vec3 ecPos,                                   \n"
"                      inout vec4 ambient,                              \n"
"                      inout vec4 diffuse,                              \n"
"                      inout vec4 specular)                             \n"
"{                                                                      \n"
"     float nDotVP;                                                     \n"
"     vec3 L = normalize(gl_ModelViewMatrix*vec4(lightPos, 0.0)).xyz;   \n"
"     nDotVP = max(0.0, dot(normal, L));                                \n"
"                                                                       \n"
"     if (nDotVP > 0.0) {                                               \n"
"       vec3 E = normalize(-ecPos);                                     \n"
"       vec3 R = normalize(reflect( L, normal ));                       \n"
"       specular = pow(max(dot(R, E), 0.0), 16.0) * lightSpecular;      \n"
"     }                                                                 \n"
"     ambient  = lightAmbient;                                          \n"
"     diffuse  = lightDiffuse * nDotVP;                                 \n"
"}                                                                      \n"
"void main() {                                                          \n"
"    vec4 ambiCol = vec4(0.0);                                          \n"
"    vec4 diffCol = vec4(0.0);                                          \n"
"    vec4 specCol = vec4(0.0);                                          \n"
"    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;          \n"
"    vec3 normal   = normalize(gl_NormalMatrix * gl_Normal);            \n"
"    vec4 ecPos    = gl_ModelViewMatrix * gl_Vertex;                    \n"
"    DirectionalLight(normal, ecPos.xyz, ambiCol, diffCol, specCol);    \n"
"    color = modelColor * (ambiCol + diffCol + specCol);                \n"
"}                                                                      \n";

static const char gFragmentShader[] =
"precision mediump float;                  \n"
"varying mediump vec4 color;               \n"
"void main() {                             \n"
"  gl_FragColor = color;                   \n"
"}                                         \n";

@implementation iphoneViewerAppDelegate

@synthesize _window;


//
//Called once app has finished launching, create the viewer then realize. Can't call viewer->run as will
//block the final inialization of the windowing system
//
- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
    //get the screen size
    CGRect lFrame = [[UIScreen mainScreen] bounds];
    
    // create the main window at screen size
    self._window = [[UIWindow alloc] initWithFrame: lFrame];
    self._window.rootViewController = [[UIViewController alloc] init];
    self._window.rootViewController.view = [[UIView alloc] initWithFrame:lFrame];
    UIView *subView =  [[UIView alloc] initWithFrame:CGRectInset(self._window.bounds,30,90)];
    subView.backgroundColor = [UIColor redColor];
    self._window.rootViewController.view.backgroundColor = [UIColor blueColor];
    [self._window.rootViewController.view addSubview:subView];
    
    //show window
    [_window makeKeyAndVisible];
    
    
    //create our graphics context directly so we can pass our own window
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    
    // Init the Windata Variable that holds the handle for the Window to display OSG in.
    osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowIOS::WindowData(subView);
    
    // Setup the traits parameters
    traits->x = 0;
    traits->y = 0;
    traits->width = subView.bounds.size.width ;
    traits->height = subView.bounds.size.height;
    traits->depth = 16; //keep memory down, default is currently 24
    //traits->alpha = 8;
    //traits->stencil = 8;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->setInheritedWindowPixelFormat = true;
    //traits->windowName = "osgViewer";
    
    traits->inheritedWindowData = windata;
    
    // Create the Graphics Context
    osg::ref_ptr<osg::GraphicsContext> graphicsContext = osg::GraphicsContext::createGraphicsContext(traits.get());
    
    //create the viewer
    _viewer = new osgViewer::Viewer();
    //if the context was created then attach to our viewer
    if(graphicsContext)
    {
        _viewer->getCamera()->setGraphicsContext(graphicsContext);
        _viewer->getCamera()->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
    }
    
    //create scene and attch to viewer
    
    //create root
    _root = new osg::MatrixTransform();
    
    osg::Geode* geode = new osg::Geode();
    osg::ShapeDrawable* drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(1,1,1), 1));
    geode->addDrawable(drawable);
    
    
    osg::Shader * vshader = new osg::Shader(osg::Shader::VERTEX, gVertexShader );
    osg::Shader * fshader = new osg::Shader(osg::Shader::FRAGMENT, gFragmentShader );
    
    osg::Program * prog = new osg::Program;
    prog->addShader ( vshader );
    prog->addShader ( fshader );
    geode->getOrCreateStateSet()->setAttribute ( prog );
    
    _root->addChild(geode);
    
    
    
    _viewer->setSceneData(_root.get());
    _viewer->setCameraManipulator(new osgGA::TrackballManipulator);
    _viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded); // SingleThreaded DrawThreadPerContext
    
    _viewer->realize();
    
    osg::setNotifyLevel(osg::DEBUG_FP);
    
    [NSTimer scheduledTimerWithTimeInterval:1.0/30.0 target:self selector:@selector(updateScene) userInfo:nil repeats:YES];
    
    //Configure and start accelerometer
    [[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0 / kAccelerometerFrequency)];
    [[UIAccelerometer sharedAccelerometer] setDelegate:self];
}


//
//Timer called function to update our scene and render the viewer
//
- (void)updateScene {
    _viewer->frame();
}


- (void)applicationWillResignActive:(UIApplication *)application {
    
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    
}


-(void)applicationWillTerminate:(UIApplication *)application{
    _root = NULL;
    _viewer = NULL;
}

//
//Accelorometer
//
- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
    //Use a basic low-pass filter to only keep the gravity in the accelerometer values
    accel[0] = acceleration.x * kFilteringFactor + accel[0] * (1.0 - kFilteringFactor);
    accel[1] = acceleration.y * kFilteringFactor + accel[1] * (1.0 - kFilteringFactor);
    accel[2] = acceleration.z * kFilteringFactor + accel[2] * (1.0 - kFilteringFactor);
}


- (void)dealloc {
    _root = NULL;
    _viewer = NULL;
    [super dealloc];
}

@end