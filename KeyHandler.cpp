// Key handler.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "KeyHandler.h"

#include "Modules/Boid/BoidsSystem.h"
#include "Modules/Dragon/Dragon.h"
#include "Modules/Island/Island.h"
#include "Modules/OscSurface/OscSurface.h"
#include "Modules/Target/Target.h"

#include <Logging/Logger.h>
#include <Devices/IMouse.h>
#include <Math/Vector.h>
#include <Sound/MusicPlayer.h>
#include <Scene/RenderStateNode.h>

#include <Display/IFrame.h>

using OpenEngine::Math::Vector;
using OpenEngine::Scene::TransformationNode;
using OpenEngine::Scene::RenderStateNode;

KeyHandler::KeyHandler(FollowCamera& camera,
                       TransformationNode& target,
                       HeightMap& hmap,
                       IMouse& mouse,
                       Island* island,
                       Dragon* dragon,
                       BoidsSystem* boidssystem,
                       TimeModifier& timeModifier,
                       GameState& gamestate,
                       MusicPlayer& musicplayer,
                       IFrame& frame,
                       RenderStateNode* rn)
    : camera(camera)
    , target(target)
    , hmap(hmap)
    , mouse(mouse)
    , timeModifier(timeModifier) 
    , gamestate(gamestate)
    , frame(frame)
    , rn(rn)
    , up(0),down(0),left(0),right(0)
    , cam_up(0),cam_down(0),cam_left(0),cam_right(0)
    , musicplayer(musicplayer)
{

  this->island = island;
  this->dragon = dragon;
  this->boidssystem = boidssystem;

  camera.Follow(&target);

  timeFactor = 1.0;
  gainStep = 0.1;
  done = pause = false;
  rotChunkMouse = 0.05;
  moveChunkMouse = 0.0015;
  rotChunkKeyboard = 0.2; //rotChunkMouse*200;
  moveChunkKeyboard = 2; //moveChunkMouse*400;
  warping = false;

  ResetCamera();
  ResetTarget();
}

void KeyHandler::ResetTarget() {
    target.SetPosition(Vector<3,float>(0,0,0));
}
void KeyHandler::ResetCamera() {
    camera.SetPosition(Vector<3,float>(0,50,100));
    camera.LookAt(0,target.GetPosition()[1],0);
}
void KeyHandler::ResetGame() {
    ResetTarget();
    ResetCamera();
    boidssystem->ResetBoids();
    timeFactor = 1.0;
    timeModifier.SetFactor(timeFactor);
    done = pause = false;
    gamestate.Reset();
}
void KeyHandler::TogglePauseGame() {
    if (pause) {
        timeModifier.SetFactor(timeFactor);
        gamestate.Unpause();
    } else {
        timeModifier.SetFactor(0.0);
        gamestate.Pause();
    }
    pause = !pause;
}

KeyHandler::~KeyHandler() {}

// set state of keys on up/down events
void KeyHandler::Handle(KeyboardEventArg arg) {
    if(arg.type == EVENT_PRESS)
      keysPressed.push_back(arg.sym);
    else {
      keysPressed.remove(arg.sym);
      HandleUp(arg.sym);
    }
}

void KeyHandler::Handle(OpenEngine::Core::ProcessEventArg arg) {

    // @todo - this should be in a init handler
    mouse.HideCursor();

    if(gamestate.GetTimeLeft() <= 0) {
        done = true;
        timeModifier.SetFactor(0.0);
    }

    list<Key>::iterator key;
    for(key=keysPressed.begin(); key != keysPressed.end(); ++key) {
        HandleDown(*key);
    }
    //keysPressed.clear(); //to disable repeatition when holding down a key

    // handle joystick.
    
    float move_factor = 2.5;

    if (up)
	MoveForward(up*move_factor);
    if (down)
	MoveBack(down*move_factor);
    if (left)
	MoveLeft(left*move_factor);
    if (right)
	MoveRight(right*move_factor);

    float zoom_factor = 3.0; 

    if (cam_up) {
	ZoomIn(cam_up*zoom_factor);
    }
    if (cam_down) {
	ZoomOut(cam_down*zoom_factor);
    }
    float rot_factor = 0.1;

    // Yeah, these ARE flippet...
    if (cam_left)
	RotateRight(cam_left*rot_factor);

    if (cam_right)
	RotateLeft(cam_right*rot_factor);

    CheckCameraCollision();
}

void KeyHandler::HandleDown(Key key) {

    switch (key) {
    case KEY_1:
      boidssystem->toggleRenderState();
        break;
    case KEY_2:
        dragon->toggleRenderState();
        break;
    case KEY_3:
        island->toggleRenderState();
        break;
    case KEY_4:
      //OscSurface::getInstance()->enableDisable();
        break;
    case KEY_5:
      //target->enableDisable();
        break;
    case KEY_6:
      //ParticleSystem::getInstance()->enableDisable();
        break;
    case KEY_8:
      boidssystem->DecNumberOfShownBoids();
        break;
    case KEY_9:
        boidssystem->IncNumberOfShownBoids();
        break;
    case KEY_a:
        MoveLeft(moveChunkKeyboard);
        break;
    case KEY_d:
        MoveRight(moveChunkKeyboard);
        break;
    case KEY_w:
        MoveForward(moveChunkKeyboard);
        break;
    case KEY_s:
        MoveBack(moveChunkKeyboard);
        break;
    case KEY_z:
        rn->ToggleOption(RenderStateNode::LIGHTING);
        rn->ToggleOption(RenderStateNode::WIREFRAME);
        break;
	/*
    case KEY_z:
      inputgrabber->rotZ = inputgrabber->rotZ - rotChunkKeyboard;
        break;
    case KEY_x:
      inputgrabber->rotZ = inputgrabber->rotZ + rotChunkKeyboard;
        break;
	*/
    case KEY_p:
        //inputgrabber->togglePause();
        break;
    case KEY_f:
        frame.ToggleOption(FRAME_FULLSCREEN);
        //inputgrabber->printLocation();
        break;
    case KEY_SPACE:
//      if( !intro->isDone() )
//          intro->disable();
        TogglePauseGame();
        break;
    case KEY_r:
        ResetGame();
        break;
    case KEY_b:
        boidssystem->ResetBoids();
        break;
    case KEY_t:
        boidssystem->DecAlignment();
        break;
    case KEY_y:
        boidssystem->IncAlignment();
        break;
    case KEY_e:
        dragon->useBreathWeapon( true );
        break;
    case KEY_q:
        dragon->ChargeFireball();
        break;
    case KEY_g:
        musicplayer.Next();
    case KEY_COMMA:
        musicplayer.SetGain(musicplayer.GetGain()-gainStep);
        break;
    case KEY_PERIOD:
        musicplayer.SetGain(musicplayer.GetGain()+gainStep);
        break;

//     case KEY_F1:
//         //inputgrabber->rotateViewAbsolute( 30, 5, 50, 1.0 );
//         target->setTarget(0, 0, 0);
//         break;
//     case KEY_F2: // ved siden af hovedet
//         //inputgrabber->rotateViewAbsolute( 25.8872, 257.27, 13.8784, 1.0 );
//         target->setTarget( -10.5859, 5.4856, -10.0325 );
//         break;
//     case KEY_F3:
//         //inputgrabber->rotateViewAbsolute( 16.1391, 335, 500, 1.0 );
//         target->setTarget(-10.5859, 5.4856, -10.0325);
//         break;
//     case KEY_F4: // inde i munden
//         //inputgrabber->rotateViewAbsolute( 54.4593, 266.533, 7.75, 1.0 );
//         target->setTarget( -2.77879, 3.63685, 0.168352);
//         break;
//     case KEY_F5: //Mellem bjergene
//         //inputgrabber->rotateViewAbsolute( 340, 245.064, 68.548, 1.0 );
//         target->setTarget( 39.9721, 6.39716, -27.3046 );
//         break;
//     case KEY_F9:
//         //inputgrabber->scaleGlobal( -0.05 );
//         break;
//     case KEY_F10:
//         //inputgrabber->scaleGlobal( 0.05 );
//         break;
    case KEY_n: //KEY_F11
         if (done) return;
         timeFactor -= 0.1;
	 if (timeFactor < 0.0) timeFactor = 0.0;
         timeModifier.SetFactor(timeFactor);
	 logger.info << "time factor: " << timeFactor << logger.end;
         break;
     case KEY_m: //KEY_F12
         if (done) return;
         timeFactor += 0.1;
	 if (timeFactor > 100.0) timeFactor = 100.0;
         timeModifier.SetFactor(timeFactor);
	 logger.info << "time factor: " << timeFactor << logger.end;
         break;
    case KEY_PAGEUP:
        ZoomIn(moveChunkKeyboard);
        break;
    case KEY_PAGEDOWN:
        ZoomOut(moveChunkKeyboard);
        break;
    case KEY_HOME:
        //inputgrabber->incMultiplier();
        break;
    case KEY_END:
        //inputgrabber->decMultiplier();
        break;
    case KEY_UP:
        RotateUp(moveChunkKeyboard);
        break;
    case KEY_DOWN:
        RotateDown(moveChunkKeyboard);
        break;
    case KEY_LEFT:
        RotateLeft(rotChunkKeyboard);
        break;
    case KEY_RIGHT:
        RotateRight(rotChunkKeyboard);
        break;
    default:
        break;
    }
}

void KeyHandler::MoveForward(float d) {
    target.Move(0, 0, -d);
}
void KeyHandler::MoveBack(float d) {
    target.Move(0, 0, d);
}
void KeyHandler::MoveLeft(float d) {
    target.Move(-d, 0, 0);
}
void KeyHandler::MoveRight(float d) {
    target.Move(d, 0, 0);
}
void KeyHandler::RotateUp(float d) {
    // check that the camera does not roll over the top
    if (0.95f < camera.GetDirection().RotateVector(Vector<3,float>(0,0,1))[1]) return;
    camera.Move(0, 0, d);
    camera.LookAt(0,target.GetPosition()[1],0);    
}
void KeyHandler::RotateDown(float d) {
    Vector<3,float> p = camera.GetPosition();
    float h = hmap.HeightAt(p)[1];
    if (h + 10 > p[1]) return;
    camera.Move(0, 0, -d);
    camera.LookAt(0,target.GetPosition()[1],0);
}
void KeyHandler::RotateRight(float d) {
    target.Rotate(0, -d, 0);
}
void KeyHandler::RotateLeft(float d) {
    target.Rotate(0, d, 0);
}
void KeyHandler::ZoomIn(float d) {
    Vector<3,float> v(target.GetPosition() - camera.GetPosition());
    if (v*v < 1000) return;
    camera.Move(d,0,0);
}
void KeyHandler::ZoomOut(float d) {
    Vector<3,float> v(target.GetPosition() - camera.GetPosition());
    if (v*v > 10000) return;
    camera.Move(-d,0,0);
}



void KeyHandler::CheckCameraCollision() {
    // if the camera is under the height map move it up
    Vector<3,float> t, c, p, r;
    t = target.GetPosition();
    p = camera.GetPosition();
    float h = hmap.HeightAt(p)[1] + 10;
    if (h > p[1]) {
        camera.Move(0, 0, h - p[1]);
        camera.LookAt(0,t[1],0);
    }
}

void KeyHandler::HandleUp(Key key) {
    switch (key) {
    case KEY_e:
        dragon->useBreathWeapon( false );
        break;
    case KEY_q:
        dragon->ShootFireball();
        break;
    default:
        break;
    }
}

void KeyHandler::Handle(JoystickButtonEventArg arg) {
    switch (arg.button) {
    case JBUTTON_TWO:
	dragon->useBreathWeapon( arg.type == JoystickButtonEventArg::PRESS);
	break;
    case JBUTTON_THREE:
        if ( arg.type == JoystickButtonEventArg::PRESS )
            dragon->ChargeFireball();
        else dragon->ShootFireball();
	break;
    case JBUTTON_NINE:
        if(arg.type == JoystickButtonEventArg::PRESS)
            ResetGame();
        break;
    case JBUTTON_TEN:
        if(arg.type == JoystickButtonEventArg::PRESS)
            TogglePauseGame();
        break;
    case JBUTTON_FIVE:
        boidssystem->DecAlignment();
        break;
    case JBUTTON_SIX:
        boidssystem->IncAlignment();
        break;
    case JBUTTON_SEVEN:
	if (arg.type == JoystickButtonEventArg::PRESS) {
        if (done) return;
	    timeFactor -= 0.1;
	    if (timeFactor < 0.0) timeFactor = 0.0;
	    timeModifier.SetFactor(timeFactor);
	    logger.info << "time factor: " << timeFactor << logger.end;
	    break;
	}
    case JBUTTON_EIGHT:
	if (arg.type == JoystickButtonEventArg::PRESS) {
        if (done) return;
	    timeFactor += 0.1;
	    if (timeFactor > 100.0) timeFactor = 100.0;
	    timeModifier.SetFactor(timeFactor);
	    logger.info << "time factor: " << timeFactor << logger.end;
	    break;
	}

    default:
	break;
    }
}

void KeyHandler::Handle(JoystickAxisEventArg arg) {

    float max = 1 << 15;
    float thres1 = 0.1;
    float thres2 = 0.15;

    up = (-arg.state.axisState[1])/max;
    if (up < thres1) up = 0.0;
    down = (arg.state.axisState[1])/max;
    if (down < thres1) down = 0.0;

    left = (-arg.state.axisState[0])/max;
    if (left < thres1) left = 0.0;
    right = (arg.state.axisState[0])/max;
    if (right < thres1) right = 0.0;

    
    cam_up = (-arg.state.axisState[3])/max;
    if (cam_up < thres2) cam_up = 0.0;
    cam_down = (arg.state.axisState[3])/max;
    if (cam_down < thres2) cam_down = 0.0;
    
    cam_left = (-arg.state.axisState[2])/max;
    if (cam_left < thres2) cam_left = 0.0;
    cam_right = (arg.state.axisState[2])/max;
    if (cam_right < thres2) cam_right = 0.0;

}
