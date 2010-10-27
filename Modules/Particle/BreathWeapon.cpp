// Breath weapon fire effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "BreathWeapon.h"

#include <Math/Vector.h>
#include <ParticleSystem/ParticleSystem.h>
#include <ParticleSystem/SimpleEmitter.h>
#include <ParticleSystem/LinearValueModifier.h>
#include <Renderers/TextureLoader.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>

using OpenEngine::Math::Vector;
using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;
using namespace OpenEngine::ParticleSystem;

BreathWeapon::BreathWeapon(OpenEngine::ParticleSystem::ParticleSystem& system,
                           HeightMap& heightMap,
                           BoidsSystem& boidssystem): 
    emitter(new SimpleEmitter(system,
                              200,     //numParticles
                              0.02,    //emitRate
                              2.1,     //life
                              0.5,     //lifeVar
                              0.09,    //angle
                              0.0,   //spin
                              0.0,   //spinVar
                              45.0,    //speed
                              5.0,     //speedVar
                              0.0,     //size
                              0.0      //sizeVar
                              )),    
            heightMod(heightMap),
            boidsMod(boidssystem, 3000) {
                
    ITexture2DPtr tex1 = 
        // ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
        ResourceManager<ITexture2D>::Create("star.jpg");
    emitter->SetTexture(tex1);
    emitter->SetGravity(Vector<3,float>(0,0.295,0));
    
    LinearValueModifier<SimpleEmitter::TYPE,Vector<4,float> >& colormod = emitter->GetColorModifier();
    LinearValueModifier<SimpleEmitter::TYPE,float>&  sizem = emitter->GetSizeModifier();
    
    // color modifier
    colormod.AddValue( .9, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    colormod.AddValue( .7, Vector<4,float>( .4,  0.3,  .1, .6)); // redish
    colormod.AddValue( .4, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .4, .3)); // blueish

    // size variations 
    sizem.AddValue(1.0, 30); 
    sizem.AddValue(.65, 25);
    // sizem.AddValue( .2, 20);    
    sizem.AddValue( .0, 20);    
    
    system.ProcessEvent().Attach(*this);
}

BreathWeapon::~BreathWeapon() {
    //system.ProcessEvent().Detach(*this);
} 

void BreathWeapon::Handle(ParticleEventArg e) {
    emitter->Handle(e);
    
    for (emitter->GetParticles()->iterator.Reset(); 
         emitter->GetParticles()->iterator.HasNext(); 
         emitter->GetParticles()->iterator.Next()) {
    
        SimpleEmitter::TYPE& particle = emitter->GetParticles()->iterator.Element();
        heightMod.Process(particle);
        boidsMod.Process(e.dt, particle);        
    }
}

ISceneNode* BreathWeapon::GetSceneNode() {
    return emitter;
}

void BreathWeapon::SetActive(bool active) {
    emitter->SetActive(active);
}
