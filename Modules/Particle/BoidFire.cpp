// Boid fire effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "BoidFire.h"

#include <Math/Vector.h>
#include <ParticleSystem/ParticleSystem.h>
#include <Renderers/TextureLoader.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>

using OpenEngine::Math::Vector;
using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;

BoidFire::BoidFire(OpenEngine::ParticleSystem::ParticleSystem& system,
                   TextureLoader& textureLoader): 
    FireEffect(system)
               // 20,     //numParticles
               // 0.04,   //emitRate
               // 3.0,    //number 
               // 2.0,    //numberVar
               // 0.3,    //life
               // 0.1,    //lifeVar
               // 0.4,    //angle
               // 200,    //spin
               // 100,    //spinVar
               // -10,    //speed
               // 0.25   //speedVar
     //antigravity
               // )    
{
    // ITexture2DPtr tex1 = 
    //     ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
    // GetEmitter()->SetTexture(tex1);
    
    // GetEmitter()->SetGravity(Vector<3,float>(0,0.182,0))


    // // color modifier
    // colormod.AddValue( .9, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    // colormod.AddValue( .2, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    // colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .3, .1)); // blueish

    // // size variations 
    // sizem.AddValue(1.0, 0.1); 
    // sizem.AddValue( .2, 1.4);    
    // sizem.AddValue( .0,   0);    

    // system.ProcessEvent().Attach(*this);
}

BoidFire::~BoidFire() {
    // system.ProcessEvent().Detach(*this);
} 

// void BoidFire::Handle(ParticleEventArg e) {
//     FireEffect::Handle(e);
//     for (particles->iterator.Reset(); 
//          particles->iterator.HasNext(); 
//          particles->iterator.Next()) {
    
//         TYPE& particle = particles->iterator.Element();
//         heightMod.Process(particle);
//         boidsMod.Process(particle);
        
//     }

// }
