// fire ball effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _DRAGON_PANIC_OE_FIRE_BALL_
#define _DRAGON_PANIC_OE_FIRE_BALL_

// #include <Effects/FireEffect.h>

#include "../Boid/BoidsSystem.h"
#include "TransformationModifier.h"
#include "HeightMapModifier.h"
#include "Explosion.h"

namespace OpenEngine {
    namespace ParticleSystem {
        class ParticleSystem;
        class ParticleEventArg;
        class SimpleEmitter;
    }

    namespace Scene {
        class ISceneNode;
    }
}

// using OpenEngine::Effects::FireEffect;
using OpenEngine::Renderers::TextureLoader;
using OpenEngine::ParticleSystem::ParticleEventArg;
using OpenEngine::ParticleSystem::SimpleEmitter;

/**
 * Fire ball particle system using OEParticleSystem
 * and Effect/FireEffect
 *
 * @class OEFireBall OEFireBall.h /DragonPanic/Modules/Particle/OEFireBall.h
 */
class OEFireBall : public IListener<ParticleEventArg> {
private:
    SimpleEmitter* emitter;
    Explosion exp;
    // TransformationModifier<SimpleEmitter::TYPE> transMod;
    bool charging, firing;
    float charge, chargeStep, initLife, initSize, initSpeed;
public:
    OEFireBall(OpenEngine::ParticleSystem::ParticleSystem& system,
               HeightMap& heightMap,
               BoidsSystem& boidsSystem);
     
    virtual ~OEFireBall();

    void Handle(ParticleEventArg e);
    void Charge();
    void Fire();
    void Reset();
    void SetTransformationNode(TransformationNode* node);
    ISceneNode* GetSceneNode();
};

#endif
