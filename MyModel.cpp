// The sample box model.  You should build a file
// very similar to this for when you make your model in
// order to plug in to the animator project.


#include "MyModel.h"

#pragma warning (disable : 4305)

/**
* Modeler calls this method once an OpenGL context becomes available,
* (which happens before the first time draw() is called), and whenever
* you click the "Reload Textures and Shaders" menu option.
*
* Make sure you call the load() methods of each of your textures and
* shaders here.
*/
void MyModel::load() {
    texture.load();
    grass.load();
    sky.load();
}

/**
* Modeler calls this method many times a second when the Animate
* checkbox is checked.
*/
void MyModel::tick() {
    // You can put code here to increment animation counters for
    // extra credit.
}

/** Draw the model. */
void MyModel::drawModel() {

    int divisions=16;
    switch(ModelerDrawState::Instance()->m_quality)
    {
    case HIGH: 
        divisions = 64; break;
    case MEDIUM: 
        divisions = 20; break;
    case LOW:
        divisions = 12; break;
    case POOR:
        divisions = 8; break;
    }

    // Remember our current modelview matrix, which is in world space.
    glPushMatrix();

    // Rotate around the X-axis
    glRotatef(rotateX.getValue(), 1, 0, 0);

    // Rotate around the Y-axis
    glRotatef(rotateY.getValue(), 0, 1, 0);

    // Lower Torso
    glTranslatef(0.0,0.8,0.0);
    drawRevolution("LowerTorso.apts",1);

    // Upper Left Leg Joint
    glPushMatrix();
    glTranslatef(-0.3,0.82,0);
    glRotatef(LHipRotateX.getValue(), 1, 0, 0);
    glRotatef(LHipRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.2);

    // Upper Left Leg
    glPushMatrix();
    glTranslatef(0.0,-2.6,0);
    drawRevolution("UpperLeg.apts",1);

    // Lower Left Leg Joint
    glPushMatrix();
    glTranslatef(0,0.925,0);
    glRotatef(LKneeRotateX.getValue(), 1, 0, 0);
    glRotatef(LKneeRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.17);

    // Lower Left Leg
    glPushMatrix();
    glTranslatef(0.0,-2.65,0);
    drawRevolution("LowerLeg.apts",1);

    // Left Foot Joint
    glPushMatrix();
    glTranslatef(0,0.95,0);
    glRotatef(90, 1, 0, 0);
    glRotatef(LFootRotateX.getValue(), 1, 0, 0);
    glRotatef(LFootRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.14);

    // Left Foot
    glPushMatrix();
    glTranslatef(0.0,-1.0, 0.0);
    drawRevolution("Foot.apts",1);
    glPopMatrix(); // Left Foot

    glPopMatrix(); // Left Foot Joint
    glPopMatrix(); // Lower Left Leg
    glPopMatrix(); // Lower Left Leg joint
    glPopMatrix(); // Upper Left Leg
    glPopMatrix(); // Upper Left Leg Joint

    // Upper Right Leg Joint
    glPushMatrix();
    glTranslatef(0.3,0.82,0);
    glRotatef(RHipRotateX.getValue(), 1, 0, 0);
    glRotatef(RHipRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.2);

    // Upper Right Leg
    glPushMatrix();
    glTranslatef(0.0,-2.6,0);
    drawRevolution("UpperLeg.apts",1);

    // Lower Right Leg Joint
    glPushMatrix();
    glTranslatef(0,0.925,0);
    glRotatef(RKneeRotateX.getValue(), 1, 0, 0);
    glRotatef(RKneeRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.17);

    // Lower Right Leg
    glPushMatrix();
    glTranslatef(0.0,-2.65,0);
    drawRevolution("LowerLeg.apts",1);

    // Right Foot Joint
    glPushMatrix();
    glTranslatef(0,0.95,0);
    glRotatef(90, 1, 0, 0);
    glRotatef(RFootRotateX.getValue(), 1, 0, 0);
    glRotatef(RFootRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.14);

    // Right Foot
    glPushMatrix();
    glTranslatef(0.0,-1.0, 0.0);
    drawRevolution("Foot.apts",1);
    glPopMatrix(); // Right Foot

    glPopMatrix(); // Right Foot Joint
    glPopMatrix(); // Lower Right Leg
    glPopMatrix(); // Lower Right Leg joint
    glPopMatrix(); // Upper Right Leg
    glPopMatrix(); // Upper Right Leg Joint


    // Waist
    glPushMatrix();
    glTranslatef(0.0,1.85,0.0);
    glRotatef(WaistRotateX.getValue(), 1, 0, 0);
    glRotatef(WaistRotateY.getValue(), 0, 1, 0);
    glRotatef(WaistRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.5);

    // Upper Torso
    glPushMatrix();
    glTranslatef(0.0,-1.55,0.0);
    drawRevolution("torso2.apts",1);

    // Neck
    glPushMatrix();
    glTranslatef(0,3.4,0);
    glRotatef(NeckRotateX.getValue(), 1, 0, 0);
    glRotatef(NeckRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.25);

    // Head
    glPushMatrix();
    glTranslatef(0,-0.9,0);
    drawRevolution("head.apts",1);
    glPopMatrix(); // Head
    glPopMatrix(); // Neck

    // Upper Left Arm Joint
    glPushMatrix();
    glTranslatef(-0.92,2.9,0);
    glRotatef(LShoulderRotateX.getValue(), 1, 0, 0);
    glRotatef(LShoulderRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.2);

    // Upper Left Arm
    glPushMatrix();
    glTranslatef(0,-3.3,0);
    drawRevolution("UpperArm.apts",1);

    // Lower Left Arm Joint
    glPushMatrix();
    glTranslatef(0,1.9,0);
    glRotatef(LElbowRotateX.getValue(), 1, 0, 0);
    glRotatef(LElbowRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.15);

    // Lower Left Arm
    glPushMatrix();
    glTranslatef(0.0,-3.3,0.0);
    drawRevolution("LowerArm.apts",1);

    // Left Hand Joint
    glPushMatrix();
    glTranslatef(0,2.1,0);
    glRotatef(LHandRotateX.getValue(), 1, 0, 0);
    glRotatef(LHandRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.15);

    // Left Hand
    glPushMatrix();
    glTranslatef(0,-1.5,0);
    drawRevolution("Hand.apts",1);
    glPopMatrix(); // Left Hand

    glPopMatrix(); // Left Hand Joint
    glPopMatrix(); // Lower Left Arm
    glPopMatrix(); // Lower Left Arm Joint
    glPopMatrix(); // Upper Left Arm
    glPopMatrix(); // Upper Left Arm Joint

    // Upper Right Arm Joint
    glPushMatrix();
    glTranslatef(0.92,2.9,0);
    glRotatef(RShoulderRotateX.getValue(), 1, 0, 0);
    glRotatef(RShoulderRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.2);

    // Upper Right Arm
    glPushMatrix();
    glTranslatef(0,-3.3,0);
    drawRevolution("UpperArm.apts",1);

    // Lower Right Arm Joint
    glPushMatrix();
    glTranslatef(0,1.9,0);
    glRotatef(RElbowRotateX.getValue(), 1, 0, 0);
    glRotatef(RElbowRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.15);

    // Lower Right Arm
    glPushMatrix();
    glTranslatef(0,-3.3,0);
    drawRevolution("LowerArm.apts",1);

    // Right Hand Joint
    glPushMatrix();
    glTranslatef(0,2.1,0);
    glRotatef(RHandRotateX.getValue(), 1, 0, 0);
    glRotatef(RHandRotateZ.getValue(), 0, 0, 1);
    drawSphere(0.15);

    // Right Hand
    glPushMatrix();
    glTranslatef(0,-1.5,0);
    drawRevolution("Hand.apts",1);
    glPopMatrix(); // Right Hand

    glPopMatrix(); // Right Hand Joint
    glPopMatrix(); // Lower Right Arm
    glPopMatrix(); // Lower Right Arm Joint
    glPopMatrix(); // Upper Right Arm
    glPopMatrix(); // Upper Right Arm Joint

    glPopMatrix(); // Upper Torso
    glPopMatrix(); // Waist

    // Go back to "world space"
    glPopMatrix();



}

/** Draw the scene. */
void MyModel::draw() {

    // Use the texture if desired.
    texture.use();

    // Call a class method that draws our model.
    drawModel();

    // This if-statement makes sure that glUseProgram is not a null
    // function pointer (which it will be if GLEW couldn't initialize).
    if (glUseProgram) {
        glUseProgram(0);
    }

    // Stop applying textures to objects
    glBindTexture(GL_TEXTURE_2D, 0);



    //
    // Draw sky
    //
    sky.use();
    glPushMatrix();
    glRotatef(90, 0, 1, 0);
    drawSphere(200);
    glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);

    //
    // Draw the floor
    //
    grass.use();

    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); // specify the texture coordinate
    glNormal3f(0, 1, 0); // specify the surface's normal at this vertex
    glVertex3f(-50, -2, -50); // both before its corresponding vertex

    glTexCoord2f(1, 1);
    glNormal3f(0, 1, 0);
    glVertex3f(50, -2, -50);

    glTexCoord2f(1, 0);
    glNormal3f(0, 1, 0);
    glVertex3f(50, -2, 50);

    glTexCoord2f(0, 0);
    glNormal3f(0, 1, 0);
    glVertex3f(-50, -2, 50);
    glEnd();

    // Stop applying textures to objects
    glBindTexture(GL_TEXTURE_2D, 0);
}

