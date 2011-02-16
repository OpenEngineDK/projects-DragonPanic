#include "HeightMap.h"

#include "../../Common/FaceListUtil.h"

#include <Core/Exceptions.h>
#include <Geometry/FaceSet.h>
#include <Renderers/IRenderingView.h>
#include <Resources/ITexture2D.h>
#include <Scene/GeometryNode.h>

#include <Logging/Logger.h>

using OpenEngine::Core::Exception;
using OpenEngine::Math::PI;
using std::min;
using std::max;

int HeightMap::Clamp(int x, int m) {
    return max(0,min(m,x));
}

HeightMap::HeightMap(UCharTexture2DPtr heightMap,
                     ITexture2DPtr texture,
                     float scale, float heightRatio, int stepSize) 
    : scale(scale), stepSize(stepSize), heightmap(heightMap) {

    HEIGHT_RATIO = heightRatio;

    if (heightMap->GetChannels() != 1)
      throw Exception("can only generate heightmap from 8bit textures");

    if(heightMap->GetWidth() != heightMap->GetHeight())
        logger.warning << "height map width != height - this has not been tested but should work!" << logger.end;

    width = heightmap->GetWidth() / stepSize;
    height = heightmap->GetHeight() / stepSize;

    // @todo: calculate this, and move it to island
    translate = Vector<3,float>(-150,-3.75,-150);

    // calculate the normalArray
    normalArray =
        new Vector<3,float>[width*height];
    CalculateNormalArray();

    // calculate and add geometry
    geometry = ConstructGeometry(texture);
    this->AddNode(geometry);
}

void HeightMap::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    VisitSubNodes(v);
}

HeightMap::~HeightMap() {
    // No need to delete `geometry' as it is a sub-node.
    delete[] normalArray;
}

/**
 * Constructs a geometry node from the scaled height map texture
 */
GeometryNode* HeightMap::ConstructGeometry(ITexture2DPtr texture) {
    int x = 0, z = 0; //Create Some Variables For Readability
    Vector<3,float> normal[4];
    Vector<3,float> point[4];
    Vector<3,float> color[4];
    Vector<3,float> tex[4];
    
    FaceSet* faces = new FaceSet();
    for ( unsigned int X = 0; X < width-1; X++ ) {
        for ( unsigned int Z = 0; Z < height-1; Z++ ) {
            for (int i=0; i<4; i++) {
                if (i==0) { x = X;   z = Z;   }
                else if (i==1) { x = X;   z = Z+1; }
                else if (i==2) { x = X+1; z = Z+1; }
                else if (i==3) { x = X+1; z = Z;   }
                else throw Exception("index out of bound");
                point[i] = Point(x,z);
                normal[i] = Normal(x,z);
                color[i] = Color(x,z);
                tex[i][0] = z*(1.0/(float)height);
                tex[i][1] = x*(1.0/(float)width);
            }
            
            Vector<3,float> diagonalCross = (point[0]-point[2]) % (point[1]-point[3]);
            bool flip = (diagonalCross[0]*diagonalCross[2]>0);
            
            FacePtr face;
            unsigned int order[] = {0,1,2,2,3,0};
            for (unsigned int i=0, t=0; i<6; i++, t=i%3) {
                int j = order[i];
                if (flip) j = (j+1)%4;
                
                //@todo do this more nice
                if (t==0) face = FacePtr(new Face(Vector<3,float>(1.0),
                                                  Vector<3,float>(0.0),
                                                  Vector<3,float>(0,0,1)));
                face->vert[t] =
                    Vector<3,float>(point[j][0],  point[j][1],  point[j][2]);


                Vector<3,float> n(normal[j][0], normal[j][1], normal[j][2]);
                face->norm[t] = n.GetNormalize();
                face->colr[t] =
                    Vector<4,float>(color[j][0], color[j][1], color[j][2], 0.0);
                face->texc[t] = Vector<2,float>(tex[j][0], tex[j][1]);
                
                if (t==2) {
                    face->mat->AddTexture(texture);
                    faces->Add(face);
                }
            }
        }
    }
    //FaceListUtil::Unitize(faces);
    //FaceListUtil::OverrideSoftNormsWithHardNorm(faces);
    return new GeometryNode(faces);
}

void HeightMap::CalculateNormalArray() {
    for (unsigned int X = 0; X < width; X++)
        for ( unsigned int Y = 0; Y < height; Y++ ) {
            normalArray[X+(Y*width)] = 
	      (Point(X+1,Y) - Point(X-1,Y))
                % (Point(X,Y+1) - Point(X,Y-1)).GetNormalize()*-1;
        }
}

/**
 * This Returns The Height From A Height Map Index
 */
float HeightMap::Height(int x, int z) {
    x = Clamp(width-1,x); // Error Check Our x Value
    z = Clamp(height-1,z); // Error Check Our y Value

    //if( !((0 < x < width) && (0 < z < height)) ) return 0.0f;

    // Index Into Our Height Array And Return The Height
    unsigned char height = heightmap->GetPixel(x * stepSize, z * stepSize)[0]; 
    return height / 255.0f;

}

/**
 * This Returns Points On The Surface
 */
Vector<3,float> HeightMap::Point(int x, int z) {
  x = Clamp(width-1,x); // Error Check Our x Value
  z = Clamp(height-1,z); // Error Check Our y Value

    return Vector<3,float>(x*1.0/(width-1),
                           Height(x,z)*HEIGHT_RATIO,
                           z*1.0/(height-1))*scale+translate;
}

float HeightMap::HeightAt(float x, float z) {
    float floatX = (x-translate[0])/scale*(width-1);
    float floatZ = (z-translate[2])/scale*(height-1);

    float xDir = fmod(floatX,1.0f);
    float zDir = fmod(floatZ,1.0f);

    // takes the height at the four surrounding points 
    // and makes a linear interpolation in both directions
    return (
            Height( ((int)floatX), (int)floatZ )*(1-xDir)*(1-zDir)+
            
            Height( ((int)floatX)+1, (int)floatZ )*xDir*(1-zDir)+
            
            Height( ((int)floatX), ((int)floatZ)+1)*(1-xDir)*zDir +
            
            Height( ((int)floatX)+1,((int)floatZ)+1 )*xDir*zDir
            
            )*HEIGHT_RATIO*scale+translate[1];
}

Vector<3,float> HeightMap::HeightAt(Vector<3,float> p) {
    return Vector<3,float>(p[0],HeightAt(p[0],p[2]),p[2]);
}

Vector<3,float> HeightMap::NormalAt(Vector<3,float> p) {
    return NormalAt(p[0],p[2]);
}

Vector<3,float> HeightMap::NormalAt(float x, float y) {
    float floatX = (x-translate[0])/scale*(width-1);
    float floatZ = (y-translate[2])/scale*(height-1);
    return (
            Normal(int(floatX)  ,int(floatZ)  )*(1-fmod(floatX,1.0f))*(1-fmod(floatZ,1.0f))+
            Normal(int(floatX)+1,int(floatZ)  )*(  fmod(floatX,1.0f))*(1-fmod(floatZ,1.0f))+
            Normal(int(floatX)  ,int(floatZ)+1)*(1-fmod(floatX,1.0f))*(  fmod(floatZ,1.0f))+
            Normal(int(floatX)+1,int(floatZ)+1)*(  fmod(floatX,1.0f))*(  fmod(floatZ,1.0f))
            ).GetNormalize();
}

float HeightMap::SlopeAt(float x, float z) {
    Vector<3,float> normal = NormalAt(x,z);
    Vector<3,float> up(0.0f, 1.0f, 0.0f);
    float dot = normal * up;
    return acos(dot);
}

/**
 * This Returns The Normal From A Normal Map Index
 */
Vector<3,float> HeightMap::Normal(int x, int z) {
  //x = Clamp(width-1,x); // Error Check Our x Value
  //z = Clamp(height-1,z); // Error Check Our y Value

    // Index Into Our Normal Array And Return The Normal
    return normalArray[x + (z * width)];
}

/**
 * Sets The Color Value For A Particular Index, Depending On The Height Index
 */
Vector<3,float> HeightMap::Color(int x, int z) {
    float fColor = Height(x,z);
    float c = cos(pow(fColor,1.0f)*PI)*-0.5+0.5;
    //return Vector<3,float>( 0.20+0.30*c, 0.15+0.35*c, 0.10+0.40*c );
    return Vector<3,float>( 0.25+0.30*c, 0.20+0.35*c, 0.15+0.40*c ) *1.5;
}
