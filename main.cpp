// Main setup for the OpenEngine DragonPanic project.
// -------------------------------------------------------------------
// Copyright (C) 2008 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

// OpenEngine stuff
#include <Meta/Config.h>

// Core structures
#include <Core/Engine.h>

// Display structures
#include <Display/IFrame.h>
#include <Display/FollowCamera.h>
#include <Display/Frustum.h>
#include <Display/InterpolatedViewingVolume.h>
#include <Display/ViewingVolume.h>
#include <Display/RenderCanvas.h>
#include <Display/OpenGL/TextureCopy.h>
// SDL implementation
#include <Display/SDLEnvironment.h>
#include <Display/HUD.h>

// Rendering structures
#include <Scene/RenderNode.h>
// OpenGL rendering implementation
#include <Renderers/OpenGL/LightRenderer.h>
// #include <Renderers/OpenGL/FBOBufferedRenderer.h>
#include <Renderers/OpenGL/Renderer.h>
#include <Renderers/OpenGL/RenderingView.h>
#include <Renderers/TextureLoader.h>
// #include <Display/OpenGL/SplitStereoCanvas.h>
// #include <Display/OpenGL/ColorStereoCanvas.h>

// Resources
#include <Resources/IModelResource.h>
#include <Resources/File.h>
#include <Resources/DirectoryManager.h>
#include <Resources/ResourceManager.h>
// OBJ and TGA plugins
#include <Resources/RAWResource.h>
#include <Resources/ITexture2D.h>
#include <Resources/SDLImage.h>
#include <Resources/OBJResource.h>

// Scene structures
#include <Scene/ISceneNode.h>
#include <Scene/SceneNode.h>
#include <Scene/GeometryNode.h>
#include <Scene/TransformationNode.h>
#include <Scene/VertexArrayTransformer.h>
#include <Scene/DisplayListTransformer.h>
#include <Scene/PointLightNode.h>

// Utilities and logger
#include <Logging/Logger.h>
#include <Logging/ColorStreamLogger.h>
#include <Utils/EventProfiler.h>
#include <Utils/FPSSurface.h>
#include <Utils/CommandLineParser.h>
#include <Scene/DotVisitor.h>

// OERacer utility files
#include <Utils/QuitHandler.h>

// sound 
#include <Sound/OpenALSoundSystem.h>
#include <Sound/MusicPlayer.h>
#include <Resources/VorbisResource.h>


//#include <Meta/GLUT.h>

#include <ParticleSystem/ParticleSystem.h>
#include <Renderers/OpenGL/ParticleRenderer.h>
#include <ParticleSystem/SimpleEmitter.h>

// from project
#include "LightFader.h"
#include "KeyHandler.h"
//#include "TransparentcyNode.h"
#include "BackgroundFader.h"
#include "GameState.h"
#include "TimeModifier.h"

#include "Modules/Island/Island.h"
#include "Modules/Island/HeightMap.h"
#include "Modules/Target/Target.h"
#include "Modules/OscSurface/OscSurface.h"
#include "Modules/Dragon/Dragon.h"
#include "Modules/Boid/BoidsSystem.h"

#include "HUD/DragonHUD.h"

// Additional namespaces
using namespace OpenEngine::Core;
using namespace OpenEngine::Logging;
using namespace OpenEngine::Devices;
using namespace OpenEngine::Display;
using namespace OpenEngine::Renderers;
using namespace OpenEngine::Renderers::OpenGL;
using namespace OpenEngine::Display::OpenGL;
using namespace OpenEngine::Resources;
using namespace OpenEngine::Utils;
using namespace OpenEngine::Sound;

// Configuration structure to pass around to the setup methods
struct Config {
    IEngine&              engine;
    EventProfiler         prof;
    IEnvironment*         env;
    IFrame*               frame;
    // Viewport*             viewport;
    IViewingVolume*       viewingvolume;
    FollowCamera*         camera;
    Frustum*              frustum;
    IRenderer*            renderer;
    IMouse*               mouse;
    IKeyboard*            keyboard;
    IJoystick*            joystick;
    ISceneNode*           scene;
    GameState*            gamestate;
    ISoundSystem*         soundsystem;
    MusicPlayer*          musicplayer;
    TimeModifier*         timeModifier;
    Target*               target;
    BoidsSystem*          boids;
    OscSurface*           oscs;
    Dragon*               dragon;
    bool                  resourcesLoaded;
    HUD*                  hud;
    OpenEngine::Renderers::TextureLoader* textureLoader;
    OpenEngine::ParticleSystem::ParticleSystem* particlesystem;
    OpenEngine::ParticleSystem::ParticleSystemTimer* pstimer;
    IRenderCanvas* canvas;
    Config(IEngine& engine)
        : engine(engine)
        , env(NULL)
        , frame(NULL)
        , viewingvolume(NULL)
        , camera(NULL)
        , frustum(NULL)
        , renderer(NULL)
        , mouse(NULL)
        , keyboard(NULL)
        , joystick(NULL)
        , scene(NULL)
        , gamestate(NULL)
        , soundsystem(NULL)
        , musicplayer(NULL)
        , timeModifier(NULL)
        , target(NULL)
        , boids(NULL)
        , oscs(NULL)
        , dragon(NULL)
        , resourcesLoaded(false)
        , hud(NULL)
        , textureLoader(NULL)
        , particlesystem(NULL)
        , pstimer(NULL)
        , canvas(NULL)
    {}
};

// Forward declaration of the setup methods
void SetupResources(Config&);
void SetupDevices(Config&);
void SetupDisplay(Config&, bool, int, int);
void SetupRendering(Config&);
void SetupParticleSystem(Config&);
void SetupSound(Config&, bool);
void SetupScene(Config&);
void SetupDebugging(Config&);


// Quick'n'dirty engine throttle. Will sleep the process event in an attempt to hit a target framerate
class EngineThrottle : public IListener<Core::ProcessEventArg> {
    float target = 60.0;
public:

    void Handle(Core::ProcessEventArg arg) {
        float dt = arg.approx / 1000000.0;
        float fps = 1.0 / dt;

        if (fps > target) {
            float targetTime = 1 / target;
            float sleepTime = targetTime - dt;
            int us = sleepTime * 1000000;

            usleep(us);
        }
    }
};

int main(int argc, char** argv) {
    // Setup logging facilities.
    Logger::AddLogger(new ColorStreamLogger(&std::cout));

    // Print usage info.
    logger.info << "========= ";
    logger.info << "Running The OpenEngine DragonPanic Project";
    unsigned int version = OE_VERSION;
    logger.info << ", version: " << version;
    logger.info << ", compiled: " << __DATE__ << " " << __TIME__;
    logger.info << " =========" << logger.end;

    logger.info << "This project is a simple testing project for OpenEngine.";
    logger.info << logger.end;

    logger.info << "  for control information see: ";
    logger.info << "KeyboardLayout.txt in the project repository" << logger.end;
    logger.info << logger.end;

    CommandLineParser clp(argc, argv);
    bool fullscreen = false;
    clp.registerModule("--fullscreen", fullscreen);
    bool sound = true;
    clp.registerModule("--nosound", sound);
    int width = 800;
    clp.registerModule("--width", width);
    int height = 600;
    clp.registerModule("--height", height);
    
    /*
    if (argc != 3) {
        logger.error << "wrong number of parameters, use fx: <program> --frame --nosound" << logger.end;
        exit(-1);
    }

    bool fullscreen = false;
    if (strcmp(argv[1],"--fullscreen") == 0)
        fullscreen = true;
    else if (strcmp(argv[1],"--frame") == 0)
        fullscreen = false;
    else {
        logger.error << "unknown frame parameter" << logger.end;
        exit(-1);
    }

    bool sound = true;
    if (strcmp(argv[2],"--nosound") == 0)
        sound = false;
    else if (strcmp(argv[2],"--sound") == 0)
        sound = true;
    else {
        logger.error << "unknown sound parameter" << logger.end;
        exit(-1);
    }
    */

    // Create an engine and config object
    Engine* engine = new Engine();
    Config config(*engine);

    // Setup the engine
    SetupResources(config);
    SetupDisplay(config, fullscreen, width, height);
    SetupDevices(config);
    SetupSound(config, sound);
    SetupParticleSystem(config);
    SetupRendering(config);
    SetupScene(config);
    
    // Possibly add some debugging stuff
    SetupDebugging(config);

    // Start up the engine.
    engine->InitializeEvent().Notify(Core::InitializeEventArg());
    logger.info << "-------------- initialization done --------------" << logger.end;



    // HACK to keep fps down
    EngineThrottle *throttle = new EngineThrottle();
    engine->ProcessEvent().Attach(*throttle);

    engine->StartMainLoop();
    engine->DeinitializeEvent().Notify(Core::DeinitializeEventArg());


#ifdef OE_DEBUG
    // Print out any profiling info
    config.prof.DumpInfo();
#endif

    // release event system
    // post condition: scene and modules are not processed
    delete engine;

    // destroy all modules
    // post condition: no one has refs to the scene
    //delete config.boids; //delete sounds
    delete config.soundsystem; // close soundsystem

    delete config.scene;

    // Return when the engine stops.
    return EXIT_SUCCESS;
}

void SetupParticleSystem(Config& config) {
    config.particlesystem = new ParticleSystem::ParticleSystem();
    
    // Add to engine for processing time (with its timer)
    //config.pstimer = 
    //new ParticleSystem::ParticleSystemTimer(*config.particlesystem);
    config.engine.InitializeEvent().Attach(*config.particlesystem);
    //config.engine.ProcessEvent().Attach(*config.pstimer);
    config.engine.DeinitializeEvent().Attach(*config.particlesystem);
}

void SetupSound(Config& config, bool sound) {
    config.soundsystem = new OpenALSoundSystem();
    config.musicplayer = new MusicPlayer(NULL,config.soundsystem);
    if (sound) {
        // setup the sound system
        config.soundsystem->SetMasterGain(1.0);
        config.engine.InitializeEvent().Attach(*config.soundsystem);
        config.engine.ProcessEvent().Attach(*config.soundsystem);
        config.engine.DeinitializeEvent().Attach(*config.soundsystem);

        // setup the music player
        config.musicplayer->AddSound("Music/beak.ogg");
        config.musicplayer->AddSound("Music/defibrilation.ogg");
        config.musicplayer->AddSound("Music/glow.ogg");
        config.musicplayer->AddSound("Music/trouble.ogg");

        config.musicplayer->SetGain(0.3);
        config.musicplayer->Shuffle(true);
        config.musicplayer->Next();
        config.musicplayer->Play();
        config.engine.ProcessEvent().Attach(*config.musicplayer);
    }
}

void SetupResources(Config& config) {
    // set the resources directory
    // @todo we should check that this path exists
    string resources = "projects/DragonPanic/data/";
    DirectoryManager::AppendPath(resources);

    // load resource plug-ins
    ResourceManager<IModelResource>::AddPlugin(new OBJPlugin());
    ResourceManager<ITexture2D>::AddPlugin(new SDLImagePlugin());
    ResourceManager<ISoundResource>::AddPlugin(new VorbisResourcePlugin());
    ResourceManager<IStreamingSoundResource>::AddPlugin(new StreamingVorbisResourcePlugin());

    config.resourcesLoaded = true;
}

void SetupDisplay(Config& config, bool fullscreen, int width, int height)
{
    if (config.frame         != NULL ||
        config.viewingvolume != NULL ||
        config.camera        != NULL ||
        config.frustum       != NULL //||
        /*config.viewport      != NULL*/)
        throw Exception("Setup display dependencies are not satisfied.");

    if (fullscreen)
        config.env = new SDLEnvironment(width, height, 32, FRAME_FULLSCREEN);
    else
        config.env = new SDLEnvironment(width, height);
    config.frame         = &config.env->CreateFrame();
    config.viewingvolume = new InterpolatedViewingVolume(*(new ViewingVolume()));
    config.camera        = new FollowCamera( *config.viewingvolume );
    //config.frustum       = new Frustum(*config.camera, 20, 3000);
}

void SetupDevices(Config& config) {
    if (config.keyboard != NULL ||
        config.mouse    != NULL ||
        config.env      == NULL)
        throw Exception("Setup devices dependencies are not satisfied.");
    // Create the mouse and keyboard input modules
    
    config.keyboard = config.env->GetKeyboard();
    config.mouse = config.env->GetMouse();
    config.joystick = config.env->GetJoystick();

    // Bind the quit handler
    QuitHandler* quit_h = new QuitHandler(config.engine);
    config.keyboard->KeyEvent().Attach(*quit_h);

    // Bind to the engine for processing time
    config.engine.InitializeEvent().Attach(*config.env);
    config.engine.ProcessEvent().Attach(*config.env);
    config.engine.DeinitializeEvent().Attach(*config.env);
}

void SetupRendering(Config& config) {
    if (config.frame == NULL ||
        config.renderer != NULL ||
        config.camera == NULL ||
        config.soundsystem == NULL )
        throw Exception("Setup renderer dependencies are not satisfied.");

    // Create a renderer
    config.renderer = new Renderer();

    // Setup a rendering view
    IRenderingView* rv = new RenderingView();
    config.renderer->ProcessEvent().Attach(*rv);

    ParticleRenderer<SimpleEmitter::TYPE>* pr = new ParticleRenderer<SimpleEmitter::TYPE>();
    config.renderer->ProcessEvent().Attach(*pr);

    // Add rendering initialization tasks
    config.textureLoader = new Renderers::TextureLoader(*config.renderer);
    config.renderer->PreProcessEvent().Attach(*config.textureLoader);

    // removed to be able to turn off textures
    //DisplayListTransformer* dlt = new DisplayListTransformer(rv);
    //config.renderer->InitializeEvent().Attach(*dlt);

    config.renderer->PreProcessEvent()
        .Attach( *(new Renderers::OpenGL::LightRenderer()) );

    // Pick a stereo mode or no stereo at all
    config.canvas = new RenderCanvas(new Display::OpenGL::TextureCopy());
    //config.canvas = new SplitStereoCanvas();
    //config.canvas = new ColorStereoCanvas();
    config.canvas->SetViewingVolume(config.camera);
    config.canvas->SetRenderer(config.renderer);
    config.frame->SetCanvas(config.canvas);
 
    config.hud = new HUD();
    config.renderer->PostProcessEvent().Attach( *config.hud );
    config.renderer->PreProcessEvent().Attach(*config.soundsystem);
}

void SetupScene(Config& config) {
    if (config.scene  != NULL ||
        config.mouse  == NULL ||
        config.keyboard == NULL ||
        config.particlesystem == NULL ||
        config.soundsystem == NULL ||
        config.resourcesLoaded == false)
        throw Exception("Setup scene dependencies are not satisfied.");

    // Create a root scene node
    RenderStateNode* renderStateNode = new RenderStateNode();
    renderStateNode->EnableOption(RenderStateNode::LIGHTING);
    renderStateNode->DisableOption(RenderStateNode::BACKFACE);
    renderStateNode->EnableOption(RenderStateNode::COLOR_MATERIAL);
    config.scene = renderStateNode;

    float fadetime = 3000.0 * 3.5;
    BackgroundFader* bgf = new BackgroundFader(fadetime, *config.renderer);
    config.engine.ProcessEvent().Attach(*bgf);

    // attach scene to soundsystem
    // config.soundsystem->SetRoot(config.scene);
    
    // Set scene lighting
    float pFade = 1.4;
    PointLightNode* light1 = new PointLightNode();
    light1->ambient = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    light1->diffuse = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    light1->specular = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    TransformationNode* light1Position = new TransformationNode();
    light1Position->SetPosition(Vector<3,float>(0.8, 0.7, 0.4));
    light1Position->AddNode(light1);
    config.scene->AddNode(light1Position);
    PointLightNode* to1 = new PointLightNode();
    to1->ambient = Vector<4,float>(0.1*pFade, 0.1*pFade, 0.1*pFade, 1.0);
    to1->diffuse = Vector<4,float>(0.4*pFade, 0.4*pFade, 0.7*pFade, 1.0);
    to1->specular = Vector<4,float>(0.0, 0.0, 0.0, 1.0);

    LightFader* light1Fader = new LightFader(light1, to1, light1, fadetime);
    config.engine.ProcessEvent().Attach(*light1Fader);

    PointLightNode* light2 = new PointLightNode();
    light2->ambient = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    light2->diffuse = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    light2->specular = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    TransformationNode* light2Position = new TransformationNode();
    light2Position->SetPosition(Vector<3,float>(-1.0, 1.0, 0.7));
    light2Position->AddNode(light2);
    config.scene->AddNode(light2Position);
    PointLightNode* to2 = new PointLightNode();
    to2->ambient = Vector<4,float>(0.1*pFade, 0.1*pFade, 0.1*pFade, 1.0);
    to2->diffuse = Vector<4,float>(0.8*pFade, 0.8*pFade, 0.7*pFade, 1.0);
    to2->specular = Vector<4,float>(0.0, 0.0, 0.0, 1.0);
    LightFader* light2Fader = new LightFader(light2, to2, light2, fadetime);
    config.engine.ProcessEvent().Attach(*light2Fader);

    TimeModifier* timeModifier = 
        new TimeModifier(config.engine.ProcessEvent(),1.00f);
    config.timeModifier = timeModifier;
    //timeModifier->SetFactor(1.23);
    timeModifier->ProcessEvent().Attach(*config.particlesystem);

    // initialize HeightMap
    string filename = DirectoryManager::FindFileInPath("Island/Terrain5.raw");
    UCharTexture2DPtr hMap = 
        UCharTexture2DPtr(new RAWResource(filename, 1024, 1024, 1));
    hMap->Load();
        
    ITexture2DPtr texture =
      ResourceManager<ITexture2D>::Create("Island/ground.tga");
    HeightMap* heightMap = new HeightMap(hMap, texture, 300.0, 0.25, 16);

    Island* island = new Island(heightMap);
    config.scene->AddNode(island);
    config.engine.InitializeEvent().Attach(*island);

    Target* target = config.target = new Target(*heightMap);
    TransformationNode* targetNode = target->GetTargetNode();
    config.scene->AddNode(targetNode);
    config.engine.ProcessEvent().Attach(*target);

    //TransparencyNode* tpNode = new TransparencyNode();
    BlendingNode* tpNode = new BlendingNode();
    config.scene->AddNode(tpNode);

    Vector<4,float> oscsColor(0.8f,0.25f,0.0f,0.7f); // lava
    //Vector<4,float> oscsColor(0.1f,0.25f,0.7f,0.7f); // water
    OscSurface* oscs = config.oscs = new OscSurface(heightMap,oscsColor);
    timeModifier->ProcessEvent().Attach(*oscs);
    tpNode->AddNode(oscs);

    // @todo: Boids have transparent shadows
    config.boids = new BoidsSystem(heightMap, oscs,*config.soundsystem, 
                                   *config.particlesystem,
                                   *config.textureLoader, config.scene);
    config.engine.InitializeEvent().Attach(*config.boids);
    timeModifier->ProcessEvent().Attach(*config.boids);
    tpNode->AddNode(config.boids);


    Dragon* dragon = config.dragon = new Dragon(heightMap, *config.boids, target,
                                                *config.textureLoader,
                                                *config.particlesystem, config.scene);
    config.scene->AddNode(dragon);
    config.engine.InitializeEvent().Attach(*dragon);
    config.engine.ProcessEvent().Attach(*dragon);

    
    // game state logic
    config.gamestate = new GameState(120);
    config.boids->BoidSystemEvent().Attach(*config.gamestate);

    config.textureLoader->Load(*config.scene);

    // Transform the scene to use vertex arrays
    VertexArrayTransformer vaT;
    vaT.Transform(*config.scene);

    // Supply the scene to the renderer
    config.canvas->SetScene(config.scene);

    // HUD
    config.textureLoader->SetDefaultReloadPolicy(Renderers::TextureLoader::RELOAD_QUEUED);
    DragonHUD* hud = new DragonHUD(*config.frame, *config.gamestate,
                                   *config.hud, *config.textureLoader);
    config.textureLoader->SetDefaultReloadPolicy(Renderers::TextureLoader::RELOAD_NEVER);
    config.engine.ProcessEvent().Attach(*hud);

    KeyHandler* key_h = new KeyHandler(*config.camera, *targetNode, *target, *heightMap,
                                       *config.mouse, island, dragon, config.boids,
                                       *timeModifier, *config.gamestate, *config.musicplayer,
                                       *config.oscs, *hud, *config.canvas, *config.frame, renderStateNode);

    config.engine.InitializeEvent().Attach(*key_h);
    config.engine.ProcessEvent().Attach(*key_h);
    config.keyboard->KeyEvent().Attach(*key_h);

    config.joystick->JoystickButtonEvent().Attach(*key_h);
    config.joystick->JoystickAxisEvent().Attach(*key_h);
}

void SetupDebugging(Config& config) {
#ifdef OE_DEBUG
    // main engine events
    config.prof.Profile<ProcessEventArg>
        ("Environment", config.engine.ProcessEvent(), *config.env);
    config.prof.Profile<ProcessEventArg>
        ("Sound System", config.engine.ProcessEvent(), *config.soundsystem);
    config.prof.Profile<ProcessEventArg>
        ("Music Player", config.engine.ProcessEvent(), *config.musicplayer);
    config.prof.Profile<ProcessEventArg>
        ("Target",       config.engine.ProcessEvent(), *config.target);
    config.prof.Profile<ProcessEventArg>
        ("Renderer",     config.engine.ProcessEvent(), *config.renderer);
    config.prof.Profile<ProcessEventArg>
        ("Dragon",       config.engine.ProcessEvent(), *config.dragon);
    // renderer events
    config.prof.Profile<RenderingEventArg>
        ("HUD", config.renderer->ProcessEvent(), *config.hud);
    // time modified events
    config.prof.Profile<ProcessEventArg>
        ("Boids System",    config.timeModifier->ProcessEvent(), *config.boids);
    config.prof.Profile<ProcessEventArg>
        ("Osc Surface",     config.timeModifier->ProcessEvent(), *config.oscs);
    config.prof.Profile<ProcessEventArg>
        ("OE Particle System", config.timeModifier->ProcessEvent(), *config.particlesystem);

    // Visualization of the frustum
    if (config.frustum != NULL) {
        config.frustum->VisualizeClipping(true);
        config.scene->AddNode(config.frustum->GetFrustumNode());
    }

    ofstream dotfile("scene.dot", ofstream::out);
    if (!dotfile.good()) {
        logger.error << "Can not open 'scene.dot' for output"
                     << logger.end;
    } else {
        DotVisitor dot;
        dot.Write(*config.scene, &dotfile);
        logger.info << "Saved scene graph to 'scene.dot'"
                    << logger.end
                    << "To create a SVG image run: "
                    << "dot -Tsvg scene.dot > scene.svg"
                    << logger.end;
    }
#endif

    // FPS layer with cairo
    FPSSurfacePtr fps = FPSSurface::Create();
    config.textureLoader->Load(fps, TextureLoader::RELOAD_QUEUED);
    config.engine.ProcessEvent().Attach(*fps);
    HUD::Surface* fpshud = config.hud->CreateSurface(fps);
    fpshud->SetPosition(HUD::Surface::LEFT, HUD::Surface::TOP); 
}

