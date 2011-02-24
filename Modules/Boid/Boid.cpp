//class header file
#include "Boid.h"

// from same module
#include "BoidsSystem.h"


// from same project
#include "../Island/HeightMap.h"
#include "../OscSurface/OscSurface.h"
#include "../../Common/VectorExt.h"
#include "../../Common/utilities.h"
#include "../../Common/OpenGLUtil.h"
#include "../Particle/BoidFire.h"

#include <math.h>

// OpenEngine Core
#include <Math/Math.h>
#include <Logging/Logger.h>
#include <Scene/ISceneNode.h>
#include <Scene/TransformationNode.h>

// extensions
#include <ParticleSystem/ParticleSystem.h>
#include <Renderers/TextureLoader.h>
#include <Sound/IMonoSound.h>

using OpenEngine::Math::PI;
using OpenEngine::Scene::TransformationNode;
using std::min;
using std::max;

Boid::Boid(HeightMap* heightMap, OscSurface* oscsurface, BoidsSystem* boidssystem,
           Vector<3,float> position, Vector<3,float> forward,
           Vector<3,float> velocity, Vector<3,float> color, IMonoSound& voice,
           OpenEngine::ParticleSystem::ParticleSystem& oeparticlesystem,
           OpenEngine::Renderers::TextureLoader& texloader,
           ISceneNode* particleRoot):
 
    particleRoot(particleRoot),
    oeparticlesystem(oeparticlesystem),
    voice(voice) {

    this->heightMap = heightMap;
    this->oscsurface = oscsurface;
    this->boidssystem = boidssystem;

    boidfire = new BoidFire(oeparticlesystem);
    boidfire->SetActive(false);
    fireTrans = new TransformationNode();
    fireTrans->AddNode(boidfire->GetSceneNode());
    particleRoot->AddNode(fireTrans);
    
    this->position = position;
    this->forward = forward.GetNormalize();
    this->velocity = velocity;
    this->color = color;
    up = Vector<3,float>(0,1,0);
    left = up%(forward);
    mass = 1.0;
    max_force = 10.0;
    max_speed = 5.0;
    steering_accumulated = Vector<3,float>(0,0,0);
    external_force_accumulated = Vector<3,float>(0,0,0);
    external_impulse_accumulated = Vector<3,float>(0,0,0);
    walkCycle = 0.0;
    airborn = false;
    prevTime = 0.0;

    burned = 0.0;
    hot = 0.0;
    burning = false;
    drowning = false;
    life = 1.0;
    dead = false;
}

Boid::~Boid(){
    // @todo how to handle deallocation of the particle renderer?
    // particleRoot->RemoveNode(boidfire->GetSceneNode());
    // delete boidfire->GetSceneNode();
    fireTrans->RemoveNode(boidfire->GetSceneNode());
    particleRoot->RemoveNode(fireTrans);
    delete boidfire;
    delete fireTrans;
}

Vector<3,float> Boid::getPosition() { return position; }
Vector<3,float> Boid::getVelocity() { return velocity; }

void Boid::addSteering( Vector<3,float> steering, double weight ) {
    Vector<3,float> vect = steering*weight;
    steering_accumulated = steering_accumulated + vect;
}

void Boid::addExternalForce( Vector<3,float> force ) {
    external_force_accumulated = external_force_accumulated + force;
}

void Boid::addExternalImpulse( Vector<3,float> impulse ) {
    external_impulse_accumulated = external_impulse_accumulated + impulse;
}

void Boid::addDesiredVelocity( Vector<3,float> steering, double weight ) {
    Vector<3,float> vect = ((steering.GetNormalize()*max_speed) - velocity )*weight;
    steering_accumulated = steering_accumulated + vect;
}
void Boid::gotoTarget( Vector<3,float> target, double radius, double weight, bool squared ) {
    Vector<3,float> targetDir = target-position;
    double targetDist = targetDir.GetLength();
    if (targetDist>0 && targetDist<radius) {
        double gotoStrength = (1/pow(targetDist,(squared?2:1)));
        addSteering(targetDir*gotoStrength,weight);
    }
}


void Boid::handleBoid(Boid* boid) {
    Vector<3,float> otherDir = boid->getPosition()-position;
    double otherDist = otherDir.GetLength();
  
    // Separation
    gotoTarget(boid->getPosition(),8.0,-8.0,true);

    // Cohesion
    //gotoTarget(boid->getPosition(),8.0,1.5,false);
    gotoTarget(boid->getPosition(),6.5+5.0* boidssystem->getAlignment(),1.5,false);
  
    // Look mostly at alignment of other boids in front
    double visualMultiplier = (forward*(otherDir.GetNormalize())+1)*0.5;

    // Alignment
    if (otherDist>0 && otherDist<8.0 && boid->getVelocity().GetLength()>0) {
        double alignmentStrength = (1/otherDist)*boidssystem->getAlignment();
        addDesiredVelocity(boid->getVelocity(),alignmentStrength*visualMultiplier);
    }
}


void Boid::update( double timeDelta ) {
    updateSteering(timeDelta);
    updatePhysics(timeDelta);
    updateLocomotion(timeDelta);
    prevTime = prevTime+timeDelta;
}

void Boid::updateSteering( double timeDelta ) {
  steering_force = VectorExt::Truncate(steering_accumulated, max_force);
    steering_accumulated = Vector<3,float>(0,0,0);
}

void Boid::updatePhysics( double timeDelta ) {
    // Gravity
    addExternalForce(Vector<3,float>(0,-20,0));
    if ((position+up).Get(1)<0.0) {
        addExternalForce(Vector<3,float>(
                              -velocity[0],
                              -velocity[1] + 22,
                              -velocity[2]
                              ));
    }

    // Add friction when not standing up on the ground (and alive)
    if (!airborn && (dead || up*(getUprightVector())<0.8)) {
        addExternalForce(velocity*-5);
    }
  
    // Add steepness force and navigation
    if (!airborn) {
      Vector<3,float> steepnessForce = heightMap->NormalAt(position);
      steepnessForce[1] = 0.0; //cpvc *Vector<3,float>(1,0,1);
        if (steepnessForce.GetLength()>0.2) {
            steepnessForce = steepnessForce.GetNormalize()*pow(steepnessForce.GetLength(),3);
            addExternalForce(steepnessForce*4);
        }
    }
    
    // Look ahead at landscape in front
    float aheadAndRight = heightMap->HeightAt(position+forward*7-left*7)[1];
    float aheadAndLeft  = heightMap->HeightAt(position+forward*7+left*7)[1];
    float aheadStraight = heightMap->HeightAt(position+forward*7       )[1];
    // Add lava repulsion (force away from areas lower than zero)
    if (aheadAndRight<0) addSteering(left*-1*-5,1.0);
    if (aheadAndLeft <0) addSteering(left   *-5,1.0);
    if (aheadStraight<0) addSteering(forward*-5,2.0);
    // Add steepness avoision (force away from areas with different height than current)
    if (Abs(aheadAndRight-position[1])>5) addSteering(left*-1*-0.005,pow(Abs(position[1]-aheadAndRight),3));
    if (Abs(aheadAndLeft -position[1])>5) addSteering(left   *-0.005,pow(Abs(position[1]-aheadAndLeft ),3));
    if (Abs(aheadStraight-position[1])>5) addSteering(forward*-0.005,pow(Abs(position[1]-aheadStraight),3));
  
    if (burning) addSteering(velocity*3,life);

    // Add steering force
    if (!airborn && !dead && up*(getUprightVector())>0.8) {
        Vector<3,float> steering_acceleration = steering_force*(1/mass)*timeDelta;
        velocity = VectorExt::Truncate((velocity+steering_acceleration),max_speed);
    }

    // Add external force
    Vector<3,float> external_acceleration = external_force_accumulated*(1/mass)*timeDelta;
    external_force_accumulated = Vector<3,float>(0,0,0);
    velocity = (velocity+external_acceleration);
    
    // Update
    Vector<3,float> positionPrev = position;
    position = position+(velocity*timeDelta);
    
    if (position[1]<heightMap->HeightAt(position)[1]+10.0*timeDelta) airborn = false; else airborn = true;
    
    Vector<3,float> onGround = heightMap->HeightAt(position);
    if (position[1]<onGround[1]) {
        position = onGround;
    }
    if (timeDelta>0 && airborn==false) velocity = (position-positionPrev)/timeDelta;

    // Add external impulse
    Vector<3,float> external_impulse = external_impulse_accumulated*(1/mass);
    if (external_impulse.GetLength()>0) {
        external_impulse_accumulated = Vector<3,float>(0,0,0);
        velocity = (velocity+external_impulse);
        position = position+(external_impulse*timeDelta);
        airborn = true;
    }
}

void Boid::updateLocomotion( double timeDelta ) {
    // TransformationNode* tn = boidfire->GetTransformationNode();
    fireTrans->SetPosition(position);
    voice.SetPosition(position);

    Vector<3,float> normal = heightMap->NormalAt(position);
    if (airborn && drowning) normal = Vector<3,float>(0,1,0);

    // Orientation
    if (airborn && !drowning) { // Falling in the air mode
        // Primary rule: local up axis approaches velocity vector:
      up = VectorExt::Truncate(up+(velocity*timeDelta*1.0),max_speed);
        up = Vector<3,float>(up[0],max(0.0f,up[1]),up[2]).GetNormalize(); // head never lower than feet
        // Secondary rule: local left axis remains like it was before
        left = (up%(forward)).GetNormalize();
        forward = (left%(up)).GetNormalize();

	//@todo add score events for flying boids
    }
    else if (dead) { // Being dead mode
        // Primary rule: local up axis approaches tangent to surface
        up = (up+(left%(normal)*timeDelta*6.0)).GetNormalize();
        // Secondary rule: local forward axis approaches pointing down in the ground
        forward = (forward+(normal*-1*timeDelta*6.0)).GetNormalize();
        left = (up%(forward)).GetNormalize();
        forward = (left%(up)).GetNormalize();
    }
    else { // Walking around mode
        // Primary rule: local up axis approaches global up axis* (*see getUprightVector())
        up = (up+(getUprightVector()*timeDelta*2.0)).GetNormalize();
        // Secondary rule: local forward axis approaches velocity vector
        forward = (forward+VectorExt::Truncate(velocity*timeDelta*3.0,max_speed)).GetNormalize();
        left = (up%(forward)).GetNormalize();
        forward = (left%(up)).GetNormalize();
    }

    running = min(1.0,velocity.GetLength()/max_speed);
    walkCycle += timeDelta*sqrt(running);
    if (dead) walkCycle = 0;

    if ((position.Get(1)<0) && (position-(velocity*timeDelta)).Get(1)>0) {
        oscsurface->createRipple(position[0],position[2],1.2,0.3*velocity.GetLength());
    }
    if ((position+up).Get(1)<0) { // in lava
        drowning = true;
        burned += 100.0;
    }
    else { // out of lava
        drowning = false;
    }
    if (burned>0) {
        hot += burned*timeDelta*0.005;
        life -= burned*timeDelta*0.0003;
        hot -= timeDelta/3.0;
        burned = 0;
    }
    if (hot>1 && life>0) {
        if (!burning) {
            voice.Play();
            boidfire->SetActive(true);
        }
        burning = true;
        
        
    }
    if (burning) {
//         OldParticle::ParticleSystem* particlesystem = boidssystem->GetParticleSystem();
//       if(particlesystem!=NULL)
// 	particlesystem->
// 	  CreateParticles(prevTime+timeDelta, prevTime, 25,
// 			  position+up, velocity, 3.0, 1.0, 1.0);
        life -= 0.05*timeDelta;
    }
    if (life<=0) {
      if(dead==false)
	boidssystem->BoidDied(*this);
      dead = true; 
    }
    if (life<-0.4) { burning = false; boidfire->SetActive(false); life = 0; }
}

Vector<3,float> Boid::GetPosition() {
  return position;
}

Vector<3,float> Boid::getUprightVector() {
    // Upright vector is global up vector
    Vector<3,float> upright = Vector<3,float>(0,1,0);
    // If running fast downwards, influence upright vector
    // with vector perpendicular to velocity, so the boid
    // runs a bit more perpenducular to the surface
    if (velocity[1]<0) {
        upright = upright+(velocity%(left)*0.2);
    }
    return upright.GetNormalize();
}

void Boid::HandleFire(Vector<3,float> firePosition, float fireStrength) {
    // Flee from fire
    gotoTarget(firePosition,16.0,-16.0*fireStrength*0.01,true);

    // Get burned
    float fireDist = (firePosition-position).GetLength();
    if (fireDist<4.0) {
        burned += (4.0/fireDist)*fireStrength;
    }
}


void Boid::draw( ) {
    // save gl state
    GLboolean t = glIsEnabled(GL_BLEND);
    GLboolean l = glIsEnabled(GL_LIGHTING);
    GLenum source, destination, equation;
    glGetIntegerv(GL_BLEND_SRC, (GLint*) &source);
    glGetIntegerv(GL_BLEND_DST, (GLint*) &destination);
    glGetIntegerv(GL_BLEND_EQUATION, (GLint*) &equation);

    glDisable(GL_LIGHTING);

    // dont use blend
    glDisable(GL_BLEND);
    draw2(false);
 
    // use blend to render shadows
    glBlendEquation(GL_MIN);
    glEnable(GL_BLEND);
    glPushMatrix();
    glColor4f( 0.0, 0.0, 0.0, 0.3 );
    Vector<3,float> n = heightMap->NormalAt(position);
    float matrix[] = {
        1, -n[0], 0,0,
        0,    1, 0,0,
        0, -n[2], 1,0,
        position[0],heightMap->HeightAt(position)[1]+0.1,position[2],1};
    glMultMatrixf(matrix);
    glScalef( 1.0, 0.0, 1.0 ); // make it flat
    draw2(true);
    glPopMatrix();

    if (!t)
        glDisable(GL_BLEND);
    if (l)
        glEnable(GL_LIGHTING);
    glBlendFunc(source, destination);
    glBlendEquation(equation);
    CHECK_FOR_GL_ERROR();
}

void Boid::draw2( bool shadow ) {
    glPushMatrix(); // Minion
  
    glTranslatef( 0.0, (1-up[1])*0.3, 0.0 );
    if (!shadow) { glTranslatef(position[0],position[1],position[2]); }
    float matrix[] = {
        left[0]    ,left[1]    ,left[2]    ,0,
        up[0]      ,up[1]      ,up[2]      ,0,
        forward[0] ,forward[1] ,forward[2] ,0,
        0         ,0         ,0         ,1};
    glMultMatrixf(matrix);
  
    // Up and down motion due to running:
    double runningSqrt = sqrt(running);
    glTranslatef( 0.0, (0.05-0.05*cos(walkCycle*8*PI+PI*3/8))*runningSqrt, 0.0 );

    Vector<3,float> shirtColor = Vector<3,float>( 0.30, 0.25, 0.20 )*0.80+color*0.20;
    float l = life;

    glPushMatrix(); // Leg
    glTranslatef( -0.15, 0.7, 0.0 );
    glRotatef((+10-45*sin(walkCycle*4*PI))*runningSqrt,1,0,0);
    glPushMatrix(); // Actual Leg
    if (!shadow) glColor3f( 0.4*l, 0.3*l, 0.2*l );
    glTranslatef( 0.0, -0.2, 0.0 );
    glScalef( 0.2, 0.6, 0.2 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPushMatrix(); // Foot
    if (!shadow) glColor3f( 0.2*l, 0.2*l, 0.2*l );  
    glTranslatef( 0.0, -0.6, 0.1 );
    glScalef( 0.2, 0.2, 0.4 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPopMatrix();
    glPushMatrix(); // Leg
    glTranslatef( 0.15, 0.7, 0.0 );  
    glRotatef((+10+45*sin(walkCycle*4*PI))*runningSqrt,1,0,0);
    glPushMatrix(); // Actual Leg
    if (!shadow) glColor3f( 0.4*l, 0.3*l, 0.2*l );
    glTranslatef( 0.0, -0.2, 0.0 );
    glScalef( 0.2, 0.6, 0.2 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPushMatrix(); // Foot
    if (!shadow) glColor3f( 0.2*l, 0.2*l, 0.2*l );
    glTranslatef( 0.0, -0.6, 0.1 );
    glScalef( 0.2, 0.2, 0.4 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPopMatrix();
    glPushMatrix(); // Arm
    glTranslatef( -0.3, 1.3, 0.0 );
    glRotatef(-90*running*running+25*sin(walkCycle*4*PI)*sin(running*PI),1,0,0);
    glPushMatrix(); // Actual arm
    if (!shadow) glColor3f( shirtColor[0]*l, shirtColor[1]*l, shirtColor[2]*l );
    glTranslatef( 0.0, -0.25, 0.0 );
    glScalef( 0.2, 0.6, 0.2 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPushMatrix(); // Hand
    if (!shadow) glColor3f( 0.7*l, 0.6*l, 0.5*l );
    glTranslatef( 0.0, -0.55, 0.0 );
    OpenGLUtil::GLSolidSphere( 0.1, 8, 4 );
    glPopMatrix();
    glPopMatrix();
    glPushMatrix(); // Arm
    glTranslatef( 0.3, 1.3, 0.0 );
    glRotatef(-90*running*running-25*sin(walkCycle*4*PI)*sin(running*PI),1,0,0);
    glPushMatrix(); // Actual arm
    if (!shadow) glColor3f( shirtColor[0]*l, shirtColor[1]*l, shirtColor[2]*l );
    glTranslatef( 0.0, -0.25, 0.0 );
    glScalef( 0.2, 0.6, 0.2 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPushMatrix(); // Hand
    if (!shadow) glColor3f( 0.7*l, 0.6*l, 0.5*l );
    glTranslatef( 0.0, -0.55, 0.0 );
    OpenGLUtil::GLSolidSphere( 0.1, 8, 4 );
    glPopMatrix();
    glPopMatrix();
    glPushMatrix(); // Body
    if (!shadow) glColor3f( shirtColor[0]*l, shirtColor[1]*l, shirtColor[2]*l );
    glTranslatef( 0.0, 1.05, 0.0 );
    glScalef( 0.5, 0.7, 0.3 );
    OpenGLUtil::GLSolidCube( 1.0 );
    glPopMatrix();
    glPushMatrix(); // Head
    if (!shadow) glColor3f( 0.7*l, 0.6*l, 0.5*l );
    glTranslatef( 0.0, 1.6, 0.0 );
    glScalef( 0.4, 0.4, 0.4 );
    OpenGLUtil::GLSolidSphere( 0.5, 12, 6 );
    glPopMatrix();
  
    glPopMatrix();
  
}

