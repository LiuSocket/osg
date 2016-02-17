#include <osg/Geode>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

static osg::Node* getScene(const std::string & ifname, const std::string & avselect)
{
    osg::Image* video;
    osgDB::Options* options = new osgDB::Options("avselect=" + avselect);
    //osgDB::Options* options = new osgDB::Options("skip_time=30 trim_duration=10 visible_time=2 start_time=4 speed=1.5 avselect=" + avselect);
    video = osgDB::readImageFile(ifname, options);
    if (!video) {
        std::cerr << "Unable to open " << ifname << std::endl;
        return NULL;
    }

    if (avselect == "video") {
        osg::Geode* geode_video = new osg::Geode;

        const int vidw = video->s(), vidh  = video->t();
        const float img_container_scale = 1.f / (vidw > vidh ? vidw : vidh);
        const float svideo_w = vidw * img_container_scale;
        const float svideo_h = vidh * img_container_scale;
        osg::Vec3 corner = osg::Vec3(     0.0,  0.0,      0.0);
        osg::Vec3 width  = osg::Vec3(svideo_w,  0.0,      0.0);
        osg::Vec3 height = osg::Vec3(     0.0,  0.0, svideo_h);
        osg::Geometry* geom_video = osg::createTexturedQuadGeometry(corner, width, height, 0.0, 1.0, 1.0, 0.0);

        osg::Texture2D* videotex = new osg::Texture2D(video);
        videotex->setResizeNonPowerOfTwoHint(false);
        geom_video->getOrCreateStateSet()->setTextureAttributeAndModes(0, videotex);
        geode_video->addDrawable(geom_video);

        return geode_video;
    } else if (avselect == "audio") {
        const char* fragment_source = "\
uniform sampler2D audiodata;\n\
varying vec2 tex_coord0;\n\
\n\
void main()\n\
{\n\
    vec2 sample_id_ch_1 = vec2(tex_coord0.x,    0.5 / 4.);\n\
    vec2 sample_id_ch_2 = vec2(tex_coord0.x,    1.5 / 4.);\n\
    vec2  power_id_ch_1 = vec2(tex_coord0.x/2., 2.5 / 4.);\n\
    vec2  power_id_ch_2 = vec2(tex_coord0.x/2., 3.5 / 4.);\n\
    float sample_ch_1 = texture2D(audiodata, sample_id_ch_1).x;\n\
    float sample_ch_2 = texture2D(audiodata, sample_id_ch_2).x;\n\
    float  power_ch_1 = texture2D(audiodata,  power_id_ch_1).x;\n\
    float  power_ch_2 = texture2D(audiodata,  power_id_ch_2).x;\n\
    power_ch_1 = sqrt(power_ch_1);\n\
    power_ch_2 = sqrt(power_ch_2);\n\
    sample_ch_1 = (sample_ch_1 + 1.) / 8.; // [-1;1] -> [0    ; 0.25]\n\
    sample_ch_2 = (sample_ch_2 + 3.) / 8.; // [-1;1] -> [0.25 ; 0.5 ]\n\
    power_ch_1 = clamp(power_ch_1, 0., 1.) / 4.; // [0 ; +oo] -> [0 ; 0.25]\n\
    power_ch_2 = clamp(power_ch_2, 0., 1.) / 4.; // [0 ; +oo] -> [0 ; 0.25]\n\
\n\
    float diff_wave_ch_1 = abs(sample_ch_1 - tex_coord0.y);\n\
    float diff_wave_ch_2 = abs(sample_ch_2 - tex_coord0.y);\n\
    if (diff_wave_ch_1 < 0.003 || diff_wave_ch_2 < 0.003) {\n\
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n\
    } else if ((tex_coord0.y > 0.75 - power_ch_1 && tex_coord0.y < 0.75) ||\n\
               (tex_coord0.y > 1.   - power_ch_2 && tex_coord0.y < 1.)) {\n\
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n\
    } else {\n\
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);\n\
    }\n\
}\n";

const char *vertex_source = "\n\
uniform mat4 osg_TextureMatrix0;\n\
\n\
varying vec2 tex_coord0;\n\
varying vec2 tex_coord1;\n\
varying vec2 quad_coord;\n\
varying vec4 color;\n\
\n\
void main(void)\n\
{\n\
    color = gl_Color;\n\
    quad_coord = gl_MultiTexCoord0.xy;\n\
    tex_coord0 = gl_MultiTexCoord0.xy;\n\
    tex_coord1 = gl_MultiTexCoord1.xy;\n\
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n\
}";

        osg::Geode* geode_audio = new osg::Geode;

        /* arbitrary visual quad */
        osg::Vec3 corner = osg::Vec3(0.0,  0.0, 0.0);
        osg::Vec3 width  = osg::Vec3(1.0,  0.0, 0.0);
        osg::Vec3 height = osg::Vec3(0.0,  0.0, 1.0);
        osg::Geometry* geom_video = osg::createTexturedQuadGeometry(corner, width, height, 0.0, 1.0, 1.0, 0.0);

        /* add shader */
        osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX);
        vertex_shader->setShaderSource(vertex_source);
        vertex_shader->setName("audio_vertex");
        osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT);
        fragment_shader->setShaderSource(fragment_source);
        fragment_shader->setName("audio_fragment");
        osg::Program* shader_program = new osg::Program();
        shader_program->setName("audio_shader");
        shader_program->addShader(vertex_shader);
        shader_program->addShader(fragment_shader);
        geode_audio->getOrCreateStateSet()->setAttributeAndModes(shader_program, true);

        /* audio data */
        osg::Texture2D* audiotex = new osg::Texture2D(video);
        audiotex->setResizeNonPowerOfTwoHint(false);
        audiotex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        audiotex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        geom_video->getOrCreateStateSet()->setTextureAttributeAndModes(0, audiotex);
        geode_audio->addDrawable(geom_video);

        osg::Uniform* audiodata = new osg::Uniform(osg::Uniform::SAMPLER_2D, "audiodata");
        geode_audio->getOrCreateStateSet()->addUniform(audiodata);

        return geode_audio;
    }

    return NULL;
}

int main(int ac, char **av)
{
    if (ac != 2 && ac != 3) {
        std::cout << "Usage: " << av[0] << " media [audio|video]" << std::endl;
        return 0;
    }
    osgViewer::Viewer viewer;
    osg::Node *node = getScene(av[1], ac == 3 ? av[2] : "video");
    if (!node)
        return 1;
    viewer.setUpViewInWindow(0, 0, 800, 600);
    viewer.setSceneData(node);

    // try lowering or raising this value (such as 15 or 60) when testing sffmpeg.
    viewer.setRunMaxFrameRate(25.);

    return viewer.run();
}
