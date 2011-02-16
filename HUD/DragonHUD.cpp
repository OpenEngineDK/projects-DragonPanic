// TimerLayer
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "DragonHUD.h"

#include <string>
#include <Math/Vector.h>
#include <Resources/ResourceManager.h>
#include <Utils/Convert.h>

using namespace OpenEngine::Math;
using namespace OpenEngine::Resources;

DragonHUD::DragonHUD(IFrame& frame, GameState& gamestate, HUD& hud, TextureLoader& texLoader)
    : gamestate(gamestate), hud(hud), texLoader(texLoader) {

    ITexture2DPtr logo = 
        ResourceManager<ITexture2D>::Create("logo/small.png");
    logo->Load();
    logoSurface = hud.CreateSurface(logo);
    logoSurface->SetPosition(HUD::Surface::RIGHT, HUD::Surface::TOP);

    // initialize a drawing tool for the red counters
    frameWidth = frame.GetWidth();
    frameHeight = frame.GetHeight();
    int textsize = frameHeight*0.08;

    // scale texture height and width to nerest power of two
    unsigned int surfWidth = 512;
    unsigned int surfHeight = textsize*2;
    unsigned int two = 2;
    while (surfHeight > two)
        two = two << 1;
    surfHeight = two;

    textTool = new CairoTextTool();
    textTool->SetFontName("Monaco");
    textTool->SetFontSize(textsize);
    textTool->Shadows(true);
    textTool->SetColor(Vector<4,float>(0.5, 0 ,0 ,0.9));

    // initialize the time counter
    timeTexture = CairoResource::Create(surfWidth,surfHeight);
    texLoader.Load(timeTexture);
    timeSurface = hud.CreateSurface(timeTexture);
    timeSurface->SetPosition(HUD::Surface::RIGHT, 
                             HUD::Surface::BOTTOM);
    int offset = 15;
    Vector<2,int> position = timeSurface->GetPosition();
    timeSurface->SetPosition( position[0]-offset,
                               position[1]-offset);

    timeleft = gamestate.GetTimeLeft();
    char timestring[255];
    int min = timeleft / 60;
    int sec =  timeleft - min*60;
    sprintf(timestring, "%d:%.2d",min,sec);
    std::string time = Convert::ToString(timestring);
    textTool->SetAlignment(CairoTextTool::RIGHT);
    textTool->DrawText(time, timeTexture);
    timeTexture->RebindTexture();

    // initialize the score counter
    pointTexture = CairoResource::Create(surfWidth,surfHeight);
    texLoader.Load(pointTexture);

    pointSurface = hud.CreateSurface(pointTexture);
    pointSurface->SetPosition(HUD::Surface::LEFT,
                              HUD::Surface::BOTTOM);
    position = pointSurface->GetPosition();
    pointSurface->SetPosition( position[0]+offset,
                               position[1]-offset);
    score = gamestate.GetScore();
    std::string points = Convert::ToString( score );
    textTool->SetAlignment(CairoTextTool::LEFT);
    textTool->DrawText(points, pointTexture);
    pointTexture->RebindTexture();
}

DragonHUD::~DragonHUD() {
  delete logoSurface;
  delete timeSurface;
  delete pointSurface;
}

void DragonHUD::Handle(OpenEngine::Core::ProcessEventArg arg) {
    if (timeleft != gamestate.GetTimeLeft()) {
      timeleft = gamestate.GetTimeLeft();
      char timestring[255];
      int min = timeleft / 60;
      int sec =  timeleft - min*60;
      sprintf(timestring, "%d:%.2d",min,sec);
      std::string time = Convert::ToString(timestring);
      textTool->SetAlignment(CairoTextTool::RIGHT);
      textTool->DrawText(time, timeTexture);
      timeTexture->RebindTexture();
    }
    if (score != gamestate.GetScore()) {
      score = gamestate.GetScore();
      std::string points = Convert::ToString( score );
      textTool->SetAlignment(CairoTextTool::LEFT);
      textTool->DrawText(points, pointTexture);
      pointTexture->RebindTexture();
    }
}

void DragonHUD::toggleRenderState() {
    hud.SetState( !hud.GetState() );
}
