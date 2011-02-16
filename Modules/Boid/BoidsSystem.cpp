#include "BoidsSystem.h"

#include "Boid.h"

#include "../Island/HeightMap.h"
#include "../OscSurface/OscSurface.h"

#include <Math/Math.h>
#include <Logging/Logger.h>
#include <ParticleSystem/ParticleSystem.h>
#include <Renderers/TextureLoader.h>
#include <Scene/ISceneNode.h>

#include <Resources/ResourceManager.h>
#include <Resources/ISoundResource.h>
#include <Sound/IMonoSound.h>

using OpenEngine::Math::PI;

BoidsSystem::BoidsSystem(HeightMap* heightMap, OscSurface* oscsurface, ISoundSystem& soundsystem,
                         OpenEngine::ParticleSystem::ParticleSystem& oeparticlesystem,
                         OpenEngine::Renderers::TextureLoader& texloader,
                         ISceneNode* particleRoot):
    textEffect(TextEffect(oeparticlesystem, texloader)),
    particleRoot(particleRoot),
    soundsystem(soundsystem),
    oeparticlesystem(oeparticlesystem),
    texloader(texloader) {
    this->heightMap = heightMap;
    this->oscsurface = oscsurface;
    enabled = true;
    numberOfRenderStates = 3;
    renderState = numberOfRenderStates-1;
    numberOfShownBoids = 0;
    disableLogic = false;
    aliveBoids = numberOfBoids;

    randGen.SeedWithTime();
    AddSoundToList("SoundFX/vester-aargh.ogg");
    AddSoundToList("SoundFX/cpvc-AARGH.ogg");
    AddSoundToList("SoundFX/ian-aargh.ogg");
    AddSoundToList("SoundFX/jakob-aargh.ogg");
    AddSoundToList("SoundFX/salomon-aargh.ogg");
    AddSoundToList("SoundFX/ptx-jargh.ogg");

    particleRoot->AddNode(textEffect.GetSceneNode());
    oeparticlesystem.ProcessEvent().Attach(textEffect);
    }


void BoidsSystem::AddSoundToList(std::string soundfile) {
    Resources::ISoundResourcePtr screamres =
        Resources::ResourceManager<Resources::ISoundResource>
        ::Create(soundfile);
    Sound::IMonoSound* screamsound = (Sound::IMonoSound*) //@todo remove cast
        soundsystem.CreateSound(screamres);
    screams.push_back(screamsound);
}

BoidsSystem::~BoidsSystem() {
    std::vector<Sound::IMonoSound*>::iterator itr;
    for (itr=screams.begin(); itr!=screams.end(); itr++)
        delete *itr;
    screams.clear();
}

void BoidsSystem::Handle(InitializeEventArg arg) {
    timer.Start();
    ResetBoids(true);
}

void BoidsSystem::ResetBoids(bool first) {
    numberOfShownBoids = numberOfBoids;
    aliveBoids = numberOfBoids;
    alignment = 0.3;
    // Init objects
    int gridSize = (int)sqrt((double)numberOfBoids);
    for (int i=0; i<gridSize; i++) {
        for (int j=0; j<gridSize; j++) {
            float val = (i+gridSize*j)*1.0/numberOfBoids;
            unsigned int index = randGen.UniformInt(0,screams.size()-1);
            if (!first)
                delete boids[i*gridSize+j];
            boids[i*gridSize+j] =
                new Boid(heightMap,oscsurface,this,
                         Vector<3,float>(i*2-(gridSize-1),0,
                                         j*2-(gridSize-1)),
                         Vector<3,float>(0,0,1),
                         Vector<3,float>(0,0,0),
                         Vector<3,float>(sin(2*PI*(0.0/3+val))*0.5+0.5,
                                         sin(2*PI*(1.0/3+val))*0.5+0.5, 
                                         sin(2*PI*(2.0/3+val))*0.5+0.5)
                         .GetNormalize(), *screams.at(index),
                         oeparticlesystem, texloader,
                         particleRoot);
        }
    }
}


void BoidsSystem::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    if (enabled) {
        // Draw boids
        for (unsigned int i=0; i<numberOfShownBoids; i++) {
            boids[i]->draw();
        }
        VisitSubNodes(v);
    }
}

void BoidsSystem::Handle(ProcessEventArg arg) {
    unsigned int dt = arg.approx;
    float deltaTime = ((float)dt)/1000.0;
    float timeStep = deltaTime / 1000.0;
    if (disableLogic) return;
    
    for (unsigned int i=0; i<numberOfShownBoids; i++) {
        for (unsigned int j=0; j<numberOfShownBoids; j++) {
            if (i!=j) boids[i]->handleBoid(boids[j]);
        }
    }
    for (unsigned int i=0; i<numberOfShownBoids; i++) {
        boids[i]->update(timeStep);
    }
}

void BoidsSystem::BoidDied(Boid& boid) {
  if (aliveBoids <= 0) {
    aliveBoids = 0;
    return;
  }
  aliveBoids--;

  Vector<3,float> position = boid.GetPosition();
  BoidSystemEventArg arg = BoidSystemEventArg(BOID_DIED, position);
  this->boidEvents.Notify(arg);

  //@TODO: use eventsystem instead???
  TransformationNode t;
  t.SetPosition(position);
  t.Rotate(PI,0,0);
  //t.SetRotation(Quaternion<float>(Vector<3,float>(0,1,0)).GetNormalize());
  textEffect.EmitText("1", &t);
}

IEvent<BoidSystemEventArg>& BoidsSystem::BoidSystemEvent() {
    return boidEvents;
}

void BoidsSystem::HandleFire(Vector<3,float> position, float strength) {
    for (int i=0; i<numberOfBoids; i++) {
        // Flee from fire
        boids[i]->HandleFire(position,strength);
    }
}

// void BoidsSystem::Handle(ParticleSystemEventArg arg) {
//     Vector<3,float> position = arg.position;
//     float strength = arg.strength;

//     for (int i=0; i<numberOfBoids; i++) {
//         // Blow away from fireball
//         Vector<3,float> dir = 
// 	  boids[i]->getPosition() - (position+Vector<3,float>(0,-5,0));

//         boids[i]->addExternalImpulse((dir.GetNormalize())*strength*
// 				     (1/std::max(3.0f,dir.GetLength())));
//     }
// }

void BoidsSystem::Explosion(Vector<3,float> position, float strength) {
//     Vector<3,float> position = arg.position;
//     float strength = arg.strength;

    for (int i=0; i<numberOfBoids; i++) {
        // Blow away from fireball
        Vector<3,float> dir = 
	  boids[i]->getPosition() - (position+Vector<3,float>(0,-5,0));

        boids[i]->addExternalImpulse((dir.GetNormalize())*strength*
				     (1/std::max(3.0f,dir.GetLength())));
    }
}


void BoidsSystem::IncNumberOfShownBoids() {
    if( numberOfShownBoids < numberOfBoids )
        numberOfShownBoids += 1;
}

void BoidsSystem::DecNumberOfShownBoids() {
    if( numberOfShownBoids > 0 )
        numberOfShownBoids -= 1;
}

void BoidsSystem::IncAlignment() {
    if( alignment < 1 ) alignment += 0.05;
    logger.info << "Boids alignment: " << alignment << logger.end;
}

void BoidsSystem::DecAlignment() {
    if( alignment > 0 ) alignment -= 0.05;
    logger.info << "Boids alignment: " << alignment << logger.end;
}

float BoidsSystem::getAlignment() { 
  return alignment; 
}

void BoidsSystem::toggleRenderState() {
    logger.info << "BoidsSystem status: ";
    enabled = true;
    renderState++;
    renderState %= numberOfRenderStates;
    switch ( renderState ){
    case 0: //without logic
        disableLogic = true;
        logger.info << "without logic";
        break;
    case 1: //disabled
        enabled = false;
        logger.info << "disabled";
        break;
    default: //render all, reset variables
        enabled = true;
        disableLogic = false;
        logger.info << "fully enabled";
        break;
    }
    logger.info << logger.end;
}
