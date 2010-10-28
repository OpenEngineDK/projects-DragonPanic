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
#include <ParticleSystem/SimpleEmitter.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>

using OpenEngine::Math::Vector;
using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;
using OpenEngine::ParticleSystem::LinearValueModifier;

BoidFire::BoidFire(OpenEngine::ParticleSystem::ParticleSystem& system): 
    emitter(new SimpleEmitter(system,
                              20,     //numParticles
                              0.04,   //emitRate
                              0.3,    //life
                              0.1,    //lifeVar
                              0.4,    //angle
                              200,    //spin
                              100,    //spinVar
                              -10,    //speed
                              0.25,   //speedVar
                              10.0,    //size
                              0.0     //sizeVar
                              ))    
{
    ITexture2DPtr tex1 = 
        // ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
        ResourceManager<ITexture2D>::Create("star.jpg");
    emitter->SetTexture(tex1);
    
    emitter->SetGravity(Vector<3,float>(0,0.182,0));

    
    LinearValueModifier<SimpleEmitter::TYPE,Vector<4,float> >& colormod = emitter->GetColorModifier();
    LinearValueModifier<SimpleEmitter::TYPE,float>&  sizem = emitter->GetSizeModifier();
    
    // color modifier
    // colormod.AddValue( .9, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    // colormod.AddValue( .2, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    // colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .3, .1)); // blueish

    colormod.AddValue( .9, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    colormod.AddValue( .7, Vector<4,float>( .4,  0.3,  .1, .6)); // redish
    colormod.AddValue( .4, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .4, .3)); // blueish

    // size variations 
    // sizem.AddValue(1.0, 0.1); 
    // sizem.AddValue( .2, 1.4);    
    // sizem.AddValue( .0,   0);    
    sizem.AddValue(1.0, 0.5); 
    sizem.AddValue( .2, 5.4);    
    sizem.AddValue( .0,   0);    

    system.ProcessEvent().Attach(*emitter);
}

BoidFire::~BoidFire() {
} 

void BoidFire::SetActive(bool active) {
    emitter->SetActive(active);
}

ISceneNode* BoidFire::GetSceneNode() {
    return emitter;
}


