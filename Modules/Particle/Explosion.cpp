// Fire ball effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "Explosion.h"

#include <ParticleSystem/ParticleSystem.h>
#include <Renderers/TextureLoader.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>


using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;
using OpenEngine::ParticleSystem::LinearValueModifier;

Explosion::Explosion(OpenEngine::ParticleSystem::ParticleSystem& system,
                     BoidsSystem& boidsSystem): 
    
    emitter(new SimpleEmitter(system,
                              200,     //numParticles
                              0.005,    //emitRate
                              0.4,     //life
                              0.1,     //lifeVar
                              2*PI,    //angle
                              0.0,     //spin
                              0.0,     //spinVar
                              20.0,      //speed
                              5.0,       //speedVar
                              0.0,      //size
                              0.0       //sizeVar
                              )),
    charge(1.0), initLife(emitter->GetLife()), initSize(3), initSpeed(emitter->GetSpeed()), 
    boidsMod(boidsSystem, 700), maxEmits(100), boidsSystem(boidsSystem)
{
    ITexture2DPtr tex1 = 
        // ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
        ResourceManager<ITexture2D>::Create("star.jpg");
    emitter->SetTexture(tex1);

    LinearValueModifier<SimpleEmitter::TYPE,Vector<4,float> >& colormod = emitter->GetColorModifier();
    LinearValueModifier<SimpleEmitter::TYPE,float>&  sizem = emitter->GetSizeModifier();
    
    
    // color modifier
    colormod.AddValue( 1.0, Vector<4,float>(0.1, 0.01, .01, .6)); // blackish
    colormod.AddValue( .8, Vector<4,float>(0.1, 0.01, .01, .6)); // blackish
    colormod.AddValue( .7, Vector<4,float>( .7,  0.3,  .1, .6)); // redish
    colormod.AddValue( .2, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .3, .1)); // blueish

    // size variations 
    sizem.AddValue(1.0, 25); 
    sizem.AddValue(.65, 40);
    sizem.AddValue( .2, 20);    
    sizem.AddValue( .0, 10);    

    t = new TransformationNode();
    tmp = new TransformationNode();

    t->AddNode(emitter);
    emitter->SetActive(false);
    system.ProcessEvent().Attach(*this);    
}

Explosion::~Explosion() {
    //system.ProcessEvent().Detach(*this);
} 

void Explosion::Fire(Vector<3,float> pos) {
    tmp->SetPosition(pos);
    boidsSystem.Explosion(pos, 200*charge);
    emitter->SetActive(true);
}

void Explosion::Handle(ParticleEventArg arg) {
    t->RemoveNode(emitter);
    tmp->AddNode(emitter);
    emitter->Handle(arg);
    tmp->RemoveNode(emitter);
    t->AddNode(emitter);
    if (emitter->GetTotalEmits() >= maxEmits) {
        emitter->SetActive(false);
        emitter->Reset();
    }
    for (emitter->GetParticles()->iterator.Reset(); 
         emitter->GetParticles()->iterator.HasNext(); 
         emitter->GetParticles()->iterator.Next()) {
        SimpleEmitter::TYPE& particle = emitter->GetParticles()->iterator.Element();
        boidsMod.Process(arg.dt, particle);
    }
}

void Explosion::SetCharge(float p) {
    emitter->SetLife(initLife + initLife * charge);
    emitter->SetSpeed(initSpeed + initSpeed * charge);
    boidsMod.SetStrength(700*charge);
}

float Explosion::GetCharge() {
    return charge;
}

ISceneNode* Explosion::GetSceneNode() {
    return t;
}
