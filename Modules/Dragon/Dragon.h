#ifndef _DRAGON_H_
#define _DRAGON_H_

// using templates
#include <Math/Vector.h>

// inherits from
#include <Core/IListener.h>
#include <Core/EngineEvents.h>
#include <Scene/RenderNode.h>
#include <Resources/ITexture2D.h>

#include <Renderers/IRenderingView.h>

#include <string>
#include <list>


class Follower;
class HeightMap;
class Target;
class Tube;
class BreathWeapon;
class BoidsSystem;
class OEFireBall;

namespace OpenEngine {
    namespace ParticleSystem {
        class ParticleSystem;
    }
    namespace Scene {
        class TransformationNode;
    }
    namespace Geometry {
      class Line;
    }
    namespace Renderers {
        class TextureLoader;
    }
}

using OpenEngine::Core::IListener;
using OpenEngine::Core::InitializeEventArg;
using OpenEngine::Core::ProcessEventArg;
using OpenEngine::Math::Vector;
using OpenEngine::Scene::RenderNode;
using OpenEngine::Renderers::RenderingEventArg;
using OpenEngine::Renderers::TextureLoader;
using OpenEngine::Scene::TransformationNode;
using OpenEngine::Scene::ISceneNodeVisitor;

using OpenEngine::Geometry::Line;
using std::list;
using std::string;

using namespace OpenEngine::Resources;

class Dragon : public IListener<OpenEngine::Core::InitializeEventArg>,
  public IListener<ProcessEventArg>, public RenderNode {
public:
    bool enabled;

    Dragon(HeightMap* heightMap, BoidsSystem& boidssystem, 
           Target* target, 
           TextureLoader& textureLoader, 
           OpenEngine::ParticleSystem::ParticleSystem& oeparticlesys, ISceneNode* particleRoot);
    
    ~Dragon();
    void toggleRenderState();
    
    void Handle(OpenEngine::Core::InitializeEventArg arg);
    void Handle(OpenEngine::Core::ProcessEventArg arg);

    virtual void Apply(RenderingEventArg arg, ISceneNodeVisitor& v);
    
    void OnLogicEnter(float timeStep);
    float jawPos;
    bool isUsingBreathWeapon();
    void useBreathWeapon( bool input );
    void ChargeFireball();
    void ShootFireball();
private:
    HeightMap* heightMap;
    Target* target;
    
    TransformationNode* headNode;
    TransformationNode* jawAngleNode;

    Tube* neck;
    ITexture2DPtr neckTexture;

    list<Line*> redlines;
    list<Line*> bluelines;
    list<Line*> greenlines;

    string folder;

    double prevTime;
    float neckLength;
    float moveNeckBack;
    Follower* headFocus;
    Follower* headPos;
    // Render flags
    unsigned int flags;

    bool enableTexture;
    int renderState, numberOfRenderStates;
    bool usingBreathWeapon, chargingFireball;
    float fireballCharge;
  
    Vector<3,float> fireSource;
    Vector<3,float> fireSourceVel;
    Vector<3,float> fireDir;

    OpenEngine::ParticleSystem::ParticleSystem& oeparticlesys;
    BreathWeapon* breathweapon;
    OEFireBall* fireball;
    ISceneNode* particleRoot;
};

#endif
