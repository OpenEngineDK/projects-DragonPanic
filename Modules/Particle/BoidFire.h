// Boid fire effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _DRAGON_PANIC_BOID_FIRE_
#define _DRAGON_PANIC_BOID_FIRE_

namespace OpenEngine {
    namespace ParticleSystem {
        class ParticleSystem;
        class SimpleEmitter;
    }
    namespace Scene {
        class ISceneNode;
    }
}

using OpenEngine::ParticleSystem::SimpleEmitter;
using OpenEngine::Scene::ISceneNode;

/**
 * Boid fire particle system using OEParticleSystem
 * and Effect/FireEffect
 *
 * @class BoidFire BoidFire.h /DragonPanic/Modules/Particle/BoidFire.h
 */
class BoidFire {
private:
    SimpleEmitter* emitter;
public:
    BoidFire(OpenEngine::ParticleSystem::ParticleSystem& system);

    virtual ~BoidFire();

    void SetActive(bool active);
    ISceneNode* GetSceneNode();
};

#endif
