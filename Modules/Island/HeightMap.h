#ifndef _HEIGHT_MAP_
#define _HEIGHT_MAP_

//inherits from
#include <Scene/RenderNode.h>

//using shared pointers
#include <Resources/Texture2D.h>

//templated classes
#include <Math/Vector.h>

// using typedefs
#include <string>

namespace OpenEngine {
  namespace Renderers {
    class IRenderingView;
  }
  namespace Scene {
    class GeometryNode;
  }
}

using OpenEngine::Math::Vector;
using OpenEngine::Renderers::RenderingEventArg;
using OpenEngine::Scene::RenderNode;
using OpenEngine::Resources::ITexture2DPtr;
using OpenEngine::Resources::UCharTexture2DPtr;
using OpenEngine::Scene::GeometryNode;
using OpenEngine::Scene::ISceneNodeVisitor;
using std::string;

class HeightMap : public RenderNode {
private:
    float scale;
    int stepSize;
    unsigned int height, width;

    Vector<3,float> translate;
    float HEIGHT_RATIO; /* Ratio That The Y Is Scaled According To The X And Z */
    GeometryNode* geometry;
    UCharTexture2DPtr heightmap;
    Vector<3,float>* normalArray; /* Holds The Normal Map Data */

    inline int Clamp(int x, int m);
    inline Vector<3,float> Point(int x, int z);
    inline Vector<3,float> Normal(int x, int z);
    inline Vector<3,float> Color(int x, int z);
    inline float Height(int x, int z);
    inline GeometryNode* ConstructGeometry(ITexture2DPtr texture);
    inline void CalculateNormalArray();

public:
    HeightMap(UCharTexture2DPtr heightMap, ITexture2DPtr texture,
              float scale, float heightRatio, int stepSize);
    virtual ~HeightMap();

    virtual void Apply(RenderingEventArg arg, ISceneNodeVisitor& v);
    
    float HeightAt(float x, float z);
    Vector<3,float> HeightAt(Vector<3,float> p);
    Vector<3,float> NormalAt(Vector<3,float> p);
};

#endif
