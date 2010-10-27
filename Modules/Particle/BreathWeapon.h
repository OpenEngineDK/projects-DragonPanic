// Breath weapon fire effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _DRAGON_PANIC_BREATH_WEAPON_
#define _DRAGON_PANIC_BREATH_WEAPON_

#include <Effects/FireEffect.h>

#include "HeightMapModifier.h"
#include "BoidsModifier.h"
#include "../Boid/BoidsSystem.h"
#include "../Island/HeightMap.h"

namespace OpenEngine {
    namespace ParticleSystem {
        class ParticleSystem;
        class ParticleEventArg;
        class SimpleEmitter;
    }

}


//using OpenEngine::ParticleSystem::ParticleSystem;
using OpenEngine::ParticleSystem::ParticleEventArg;
using OpenEngine::ParticleSystem::SimpleEmitter;

/**
 * Breath Weapon particle system using OEParticleSystem
 * and Effect/FireEffect
 *
 * @class BreathWeapon BreathWeapon.h /DragonPanic/Modules/Particle/BreathWeapon.h
 */
class BreathWeapon : public IListener<ParticleEventArg> {
private:
    SimpleEmitter* emitter;
    HeightMapModifier<SimpleEmitter::TYPE> heightMod;
    BoidsModifier<SimpleEmitter::TYPE> boidsMod;
public:
    BreathWeapon(OpenEngine::ParticleSystem::ParticleSystem& system,
                 HeightMap& heightMap,
                 BoidsSystem& boidsSystem);
    
    virtual ~BreathWeapon();

    void Handle(ParticleEventArg e);

    ISceneNode* GetSceneNode();
    void SetActive(bool active);

};

#endif
