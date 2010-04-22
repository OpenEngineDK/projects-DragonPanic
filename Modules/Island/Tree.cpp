#include "Tree.h"

#include <Meta/OpenGL.h>
#include "../../Common/OpenGLUtil.h"
#include <Meta/GLUT.h>
#include <Scene/RenderNode.h>
#include <Renderers/IRenderingView.h>

Tree::Tree(Vector<3,float> position) {
  this->position = position;
}

Tree::~Tree() {
}

void Tree::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    glColor3f( 0.25, 0.35, 0.10 );

    glPushMatrix();
    glTranslatef( position[0], position[1]+2, position[2] );
    glScalef(1,1.5,1);
    OpenGLUtil::GLSolidSphere( 1.0, 8, 4 );
    glPopMatrix();

    glColor3f( 0.33, 0.22, 0.11 );

    glPushMatrix();
    glTranslatef( position[0], position[1]+0.4, position[2] );
    glScalef(1,1.5,1);
    OpenGLUtil::GLSolidCube( 0.6 );
    glPopMatrix();

    VisitSubNodes(v);
}
