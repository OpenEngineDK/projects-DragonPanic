#include "Tube.h"

#include "../../Common/VectorExt.h"
#include "../../Common/utilities.h"
#include "../../Common/OpenGLUtil.h"

#include <Math/Math.h>
#include <math.h>

using OpenEngine::Math::PI;
using std::min;
using std::max;

Tube::Tube(float radius1, float radius2, float stiffness1, 
	   float stiffness2, float length) {
    this->length = length;
    this->radius1 = radius1;
    this->radius2 = radius2;
    this->stiffness1 = stiffness1;
    this->stiffness2 = stiffness2;
    adjuster = 10.0;
    twistAngle = 0.0;
}

Tube::~Tube() {
}

float Tube::getLinkRadius(int linkIndex) { 
  return linkR[linkIndex]; 
}

Vector<3,float> Tube::getLinkX(int linkIndex) {
  return linkX[linkIndex];
}

Vector<3,float> Tube::getLinkY(int linkIndex) { 
  return linkY[linkIndex]; 
}

Vector<3,float> Tube::getLinkZ(int linkIndex) { 
  return linkZ[linkIndex]; 
}

Vector<3,float> Tube::getLinkPosition(int linkIndex) { 
  return linkP[linkIndex]; 
}

float* Tube::getLinkMatrix(int linkIndex) { 
  return linkM[linkIndex]; 
}

Vector<3,float> Tube::spline(float s) { //bezier spline, with 4 control points
  //s value from 0-1

    float t = 1-s;

    Vector<3,float> v01 = p1*t+p0*s;
    Vector<3,float> v12 = p0*t+p3*s;
    Vector<3,float> v23 = p3*t+p2*s;

    Vector<3,float> v012 = v01*t+v12*s;
    Vector<3,float> v123 = v12*t+v23*s;


    return v012*t+v123*s; //returns a point on the spline
}

void Tube::update(Vector<3,float> pointA, Vector<3,float> dirA,
		  Vector<3,float> upA, Vector<3,float> pointB, 
		  Vector<3,float> dirB, Vector<3,float> upB, 
		  bool updateTwisting) {
  this->p1 = pointA;
  this->p2 = pointB;
  this->z1 = dirA;
  this->z2 = dirB;
  this->y1 = upA;
  this->y2 = upB;
  
  float curlen = 0.0;
  Vector<3,float> pA, pB;
  
  if ((p2-p1).GetLength()>length) {
    p2 = p1+(p2-p1).GetNormalize()*length*0.999999;
  }
  
  adjuster = 10.0;
  
  int _m = 100;
  while (pow(curlen-length,2)>length*0.0001) {
    if (adjuster==0) break;
    
    // Recalculate tangent points with new _m value.
    this->p0 = (z1.GetNormalize()* 1*stiffness1*length*adjuster)+p1;
    this->p3 = (z2.GetNormalize()*-1*stiffness2*length*adjuster)+p2;
    
    pB = p1; // First point in spline spline.
    curlen = 0; // Reset calculated length.
    
    // Loop through all the points:
    // Run through all points but the first.
    for (int c=1; c<samples; c++) {
      
      // Calculate point on spine:
      pA = spline(c*1.0/(samples-1.0));
      
      // The length of the current segment is
      // calculated and added to the whole length:
      curlen += (pA-pB).GetLength();
      
      pB = pA;
    }
    
    // Compare calculated length with desired length and adjust adjuster.
    adjuster = adjuster*length/curlen;
    
    _m--;
    if (_m<0) break;
  }
  
  // Calculate length from the start point to each sample point:
  sample[0] = 0;
  pB = p1;
  for (int c=1; c<samples; c++) {
    pA = spline(c*1.0/(samples-1.0));
    sample[c] = sample[c-1]+((pB-pA).GetLength());
    pB = pA;
  }
  // Convert sample values to T values between 0 and 1:
  for (int c=0; c<samples; c++) {
    sample[c] = sample[c]/sample[samples-1];
  }
  
  // If start point radius and end point radius is equal,
  // distribute points evenly:
  if (radius1==radius2) {

    // Run through all points.
    for (int c=0; c<links; c++) {
      // All radii equals to the first radii.
      linkR[c] = radius1; 

      // Lineary distributed T values.
      linkT[c] = c*1.0/(links-1);
    }  
  } else { // Distribute T values exponentialy increasingly:

    // One sphere's radius relative to the previous sphere's radius:
    float growth = pow(radius2*1.0/radius1,1.0/(links-1.0));

    // Run through all points but the last.
    for (int c=0; c<links-1; c++) { 
      // Radius for current point.
      linkR[c] = radius1*1.0*pow(growth*1.0,c*1.0);

      // Calculate T value based on radius:
      linkT[c] = (linkR[c]+0.0-radius1)/(radius2+0.0-radius1);
    }
  }
  
  // Radius for last point.
  linkR[links-1] = radius2; 
  
  // T value for last point is always 1.
  linkT[links-1] = 1;
  
  int _c = 0; // Counter for points.
  int _d = 0; // Counter for ranges (see below).
  float desired;
  
  // Calculate points
  while (_c<links) { // Run through all points.
    // The T value that must be found for the current point.
    desired = linkT[_c];

    // If the right T value is in the range between the two sample T values...
    if (desired>=sample[_d] && desired<=sample[_d+1]) {

      // ...estimate best S value inside range
      // then calculate spline point from that value...
      linkP[_c] = spline(
			 (
			  _d
			  +((desired-sample[_d])/(sample[_d+1]-sample[_d]))
			  )/(samples-1)
			 );
      
      _c++; // ...and go to next point.
    } else {
      _d++; // ...else, go to next range.
    }
  }
  
  // Calculate transformations
  linkZ[0] = z1.GetNormalize();
  linkY[0] = (linkZ[0]%(y1%(linkZ[0]))).GetNormalize();
  linkZ[links-1] = z2.GetNormalize();
  
  for (int c=1; c<links; c++) {
    // Calculate z vector for current point by
    // looking at previous point and next point:
    if (c<links-1)
      linkZ[c] = (linkP[c+1]-linkP[c-1]).GetNormalize();
    
    // Calculate current y vector, but without the twisting:
    if ((linkZ[c]-linkZ[c-1]).GetLength()>0) {
      /*linkY[c] = VectorExt::
	RotateAround((linkY[c-1].GetNormalize()),
		     linkZ[c-1]%(linkZ[c]),
		     acos(linkZ[c]*(linkZ[c-1]))
		     );*/
		linkY[c] = linkZ[c] % (linkY[c-1]%linkZ[c]);
    } else {
      linkY[c] = linkY[c-1];
    }
  }
  
  if (updateTwisting) {
    // Calculate angle the neck must be twisted.
    Vector<3,float> y2_corrected = 
      (linkZ[links-1]%(y2%(linkZ[links-1]))).GetNormalize(); // corrected y2
    
    twistAngle = 
      acos(max(-1.0f,min(1.0f,
			 y2_corrected*(linkY[links-1].GetNormalize()))));
    
    // Checks if the neck should be twisted clockwise or anticlockwise:
    Vector<3,float> clockwise = VectorExt::
      RotateAround(linkY[links-1],linkZ[links-1],+twistAngle);
    Vector<3,float> anticwise = VectorExt::
      RotateAround(linkY[links-1],linkZ[links-1],-twistAngle);

    if ( (clockwise-y2).GetLength() > (anticwise-y2).GetLength() )
      twistAngle = -twistAngle;

    if (Abs(twistAngle) > PI*0.5)
      twistAngle = sin(twistAngle)*PI*0.5;
  }
  
  for (int c=0; c<links; c++) { // Run through all points.
    
    // Corrects y vector with apropriate twisting:
    linkY[c] = VectorExt::
      RotateAround(linkY[c], linkZ[c], twistAngle*(c/(links-1.0)) );
    
    // Calculates x vector:
    linkX[c] = (linkY[c]%(linkZ[c])).GetNormalize();
    
    // Transformation for each link:
    float matrix[] = {
      linkX[c][0],linkX[c][1],linkX[c][2],0,
      linkY[c][0],linkY[c][1],linkY[c][2],0,
      linkZ[c][0],linkZ[c][1],linkZ[c][2],0,
      linkP[c][0],linkP[c][1],linkP[c][2],1
    };
    for (int i=0; i<16; i++) {
      linkM[c][i] = matrix[i];
    }
  }
}

void Tube::draw() {
  // Draw fins
  glColor3f( 0.2, 0.8, 0.2 );
  for (int c=2; c<links; c+=3) {
    glPushMatrix();
    glMultMatrixf(linkM[c]);
    glScalef( linkR[c], linkR[c], linkR[c] );
    glTranslatef( 0, 1, 0 );
    glRotatef( -40, 1, 0, 0 );
    glScalef( 0.05, 0.75, 0.35 );
    OpenGLUtil::GLSolidSphere( 1.0, 8, 4 );
    glPopMatrix();
  }
  
  // Draw tube
  //  glColor3f( 0.2, 0.55, 0.2 );
  glBegin( GL_QUADS );
  for (int c=0; c<links-1; c++) {
    Vector<3,float> o1 = linkP[c  ];
    Vector<3,float> o2 = linkP[c+1];
    float r1 = linkR[c  ];
    float r2 = linkR[c+1];
    
    for (int r=0; r<8; r++) {
      float rot1 =  r   /8.0;
      float rot2 = (r+1)/8.0;
      Vector<3,float> n11 = 
	linkX[c  ]*sin(rot1*2*PI)+linkY[c  ]*cos(rot1*2*PI);
      Vector<3,float> n12 = 
	linkX[c  ]*sin(rot2*2*PI)+linkY[c  ]*cos(rot2*2*PI);
      Vector<3,float> n21 = 
	linkX[c+1]*sin(rot1*2*PI)+linkY[c+1]*cos(rot1*2*PI);
      Vector<3,float> n22 = 
	linkX[c+1]*sin(rot2*2*PI)+linkY[c+1]*cos(rot2*2*PI);
      
      glTexCoord2f((r)/8.0, (c+1)/(links-1.0));
      glNormal3f(n21[0], n21[1], n21[2]);
      glVertex3f(n21[0]*r2+o2[0], n21[1]*r2+o2[1], n21[2]*r2+o2[2]);

      glTexCoord2f((r+1)/8.0, (c+1)/(links-1.0));
      glNormal3f(n22[0], n22[1], n22[2]);
      glVertex3f(n22[0]*r2+o2[0], n22[1]*r2+o2[1], n22[2]*r2+o2[2]);
      
      glTexCoord2f((r+1)/8.0, (c)/(links-1.0));
      glNormal3f(n12[0], n12[1], n12[2]);
      glVertex3f(n12[0]*r1+o1[0], n12[1]*r1+o1[1], n12[2]*r1+o1[2]);
      
      glTexCoord2f((r)/8.0, (c)/(links-1.0));
      glNormal3f(n11[0], n11[1], n11[2]);
      glVertex3f(n11[0]*r1+o1[0], n11[1]*r1+o1[1], n11[2]*r1+o1[2]);
      
    }
  }
  glEnd();
}
