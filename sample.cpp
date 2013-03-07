// The sample box model.  You should build a file
// very similar to this for when you make your model in
// order to plug in to the animator project.

#pragma warning (disable : 4305)
#pragma warning (disable : 4244)
#pragma warning (disable : 4786)
#pragma warning (disable : 4312)

#include "modelerview.h"
#include "modelerdraw.h"
#include "modelerui.h"
#include "vault.h"
#include "FL/glut.h"
#include "model.h"
#include "cameras.h"
#include <cmath>


void ground(float h);
void base(float h);
void rotation_base(float h);
void lower_arm(float h);
void upper_arm(float h);
void claw(float h);
void y_box(float h);

class RobotArm : public Model
{
	RangeProperty baseRotation, lowerTilt, upperTilt, clawRotation, baseLength,
		lowerLength, upperLength;

public:
    RobotArm() :
		Model("Robot Arm"),
		baseRotation("base rotation (theta)", -180, 180, 0,0.1),
		lowerTilt("lower arm tilt (phi)", 15, 95, 55, .1),
		upperTilt("upper arm tilt (psi)", 0, 135, 30, .1),
		clawRotation("claw rotation (cr)", -30, 180, 0, .1),
		baseLength("base height (h1)", .5, 10, .8, .1),
		lowerLength("lower arm length (h2)", 1, 10, 3, .1),
		upperLength("upper arm length (h3)", 1, 10, 2.5, .1)
	{
		properties.add(&baseRotation)
				  .add(&lowerTilt)
				  .add(&upperTilt)
				  .add(&clawRotation)
				  .add(&baseLength)
				  .add(&lowerLength)
				  .add(&upperLength);
	}
	virtual void draw();
};


// We are going to override (is that the right word?) the draw()
// method of ModelerView to draw out RobotArm
void RobotArm::draw()
{
	/* pick up the slider values */
	float theta = baseRotation.getValue();
	float phi = lowerTilt.getValue();
	float psi = upperTilt.getValue();
	float cr = clawRotation.getValue();
	float h1 = baseLength.getValue();
	float h2 = lowerLength.getValue();
	float h3 = upperLength.getValue();




	static GLfloat lmodel_ambient[] = {0.4,0.4,0.4,1.0};

	// define the model

	ground(-0.2);

	base(0.8);

    glTranslatef( 0.0, 0.8, 0.0 );			// move to the top of the base
    glRotatef( theta, 0.0, 1.0, 0.0 );		// turn the whole assembly around the y-axis. 
	rotation_base(h1);						// draw the rotation base

    glTranslatef( 0.0, h1, 0.0 );			// move to the top of the base
    glRotatef( phi, 0.0, 0.0, 1.0 );		// rotate around the z-axis for the lower arm
	glTranslatef( -0.1, 0.0, 0.4 );
	lower_arm(h2);							// draw the lower arm

    glTranslatef( 0.0, h2, 0.0 );			// move to the top of the lower arm
    glRotatef( psi, 0.0, 0.0, 1.0 );		// rotate  around z-axis for the upper arm
	upper_arm(h3);							// draw the upper arm

	glTranslatef( 0.0, h3, 0.0 );
	glRotatef( cr, 0.0, 0.0, 1.0 );
	claw(1.0);

}

void ground(float h) 
{
	glDisable(GL_LIGHTING);
	glColor3f(0.65,0.45,0.2);
	glPushMatrix();
	glScalef(30,0,30);
	y_box(h);
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

void base(float h) {
	setDiffuseColor( 0.25, 0.25, 0.25 );
	setAmbientColor( 0.25, 0.25, 0.25 );
	glPushMatrix();
		glPushMatrix();
			glTranslatef(1.0, h / 2.0, 0.75);
			drawCylinder(0.25, h / 2.0, h / 2.0);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(1.0, h / 2.0, -1.0);
			drawCylinder(0.25, h / 2.0, h / 2.0);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(-1.0, h / 2.0, 0.75);
			drawCylinder(0.25, h / 2.0, h / 2.0);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(-1.0, h / 2.0, -1.0);
			drawCylinder(0.25, h / 2.0, h / 2.0);
		glPopMatrix();
	glScalef(4.0f, h, 4.0f);
	y_box(1.0f);
	glPopMatrix();
}

void rotation_base(float h) {
	setDiffuseColor( 0.85, 0.75, 0.25 );
	setAmbientColor( 0.95, 0.75, 0.25 );
	glPushMatrix();
		glPushMatrix();
			glScalef(4.0, h, 4.0);
			y_box(1.0f); // the rotation base
		glPopMatrix();
		setDiffuseColor( 0.15, 0.15, 0.65 );
		setAmbientColor( 0.15, 0.15, 0.65 );
		glPushMatrix();
			glTranslatef(-0.5, h, -0.6);
			glScalef(2.0, h, 1.6);
			y_box(1.0f); // the console
		glPopMatrix();
		setDiffuseColor( 0.65, 0.65, 0.65 );
		setAmbientColor( 0.65, 0.65, 0.65 );
		glPushMatrix();
			glTranslatef( 0.5, h, 0.6 );
			glRotatef( -90.0, 1.0, 0.0, 0.0 );
			drawCylinder( h, 0.05, 0.05 ); // the pipe
		glPopMatrix();
	glPopMatrix();
}

void lower_arm(float h) {					// draw the lower arm
	setDiffuseColor( 0.85, 0.75, 0.25 );
	setAmbientColor( 0.95, 0.75, 0.25 );
	y_box(h);
}

void upper_arm(float h) {					// draw the upper arm
	setDiffuseColor( 0.85, 0.75, 0.25 );
	setAmbientColor( 0.95, 0.75, 0.25 );
	glPushMatrix();
	glScalef( 1.0, 1.0, 0.7 );
	y_box(h);
	glPopMatrix();
}

void claw(float h) {
	setDiffuseColor( 0.25, 0.25, 0.85 );
	setAmbientColor( 0.25, 0.25, 0.85 );

	glBegin( GL_TRIANGLES );

	glNormal3d( 0.0, 0.0, 1.0);		// +z side
	glVertex3d( 0.5, 0.0, 0.5);
	glVertex3d(-0.5, 0.0, 0.5);
	glVertex3d( 0.5,   h, 0.5);

	glNormal3d( 0.0, 0.0, -1.0);	// -z side
	glVertex3d( 0.5, 0.0, -0.5);
	glVertex3d(-0.5, 0.0, -0.5);
	glVertex3d( 0.5,   h, -0.5);

	glEnd();

	glBegin( GL_QUADS );

	glNormal3d( 1.0,  0.0,  0.0);	// +x side
	glVertex3d( 0.5, 0.0,-0.5);
	glVertex3d( 0.5, 0.0, 0.5);
	glVertex3d( 0.5,   h, 0.5);
	glVertex3d( 0.5,   h,-0.5);

	glNormal3d( 0.0,-1.0, 0.0);		// -y side
	glVertex3d( 0.5, 0.0, 0.5);
	glVertex3d( 0.5, 0.0,-0.5);
	glVertex3d(-0.5, 0.0,-0.5);
	glVertex3d(-0.5, 0.0, 0.5);

	glEnd();
}

void y_box(float h) {

	glBegin( GL_QUADS );

	glNormal3d( 1.0 ,0.0, 0.0);			// +x side
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d( 0.25,0.0,-0.25);
	glVertex3d( 0.25,  h,-0.25);
	glVertex3d( 0.25,  h, 0.25);

	glNormal3d( 0.0 ,0.0, -1.0);		// -z side
	glVertex3d( 0.25,0.0,-0.25);
	glVertex3d(-0.25,0.0,-0.25);
	glVertex3d(-0.25,  h,-0.25);
	glVertex3d( 0.25,  h,-0.25);

	glNormal3d(-1.0, 0.0, 0.0);			// -x side
	glVertex3d(-0.25,0.0,-0.25);
	glVertex3d(-0.25,0.0, 0.25);
	glVertex3d(-0.25,  h, 0.25);
	glVertex3d(-0.25,  h,-0.25);

	glNormal3d( 0.0, 0.0, 1.0);			// +z side
	glVertex3d(-0.25,0.0, 0.25);
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d( 0.25,  h, 0.25);
	glVertex3d(-0.25,  h, 0.25);

	glNormal3d( 0.0, 1.0, 0.0);			// top (+y)
	glVertex3d( 0.25,  h, 0.25);
	glVertex3d( 0.25,  h,-0.25);
	glVertex3d(-0.25,  h,-0.25);
	glVertex3d(-0.25,  h, 0.25);

	glNormal3d( 0.0,-1.0, 0.0);			// bottom (-y)
	glVertex3d( 0.25,0.0, 0.25);
	glVertex3d(-0.25,0.0, 0.25);
	glVertex3d(-0.25,0.0,-0.25);
	glVertex3d( 0.25,0.0,-0.25);

	glEnd();
}

/** The scene, which includes the lights and models. */
class Scene : public Model {
protected:
///////////////////////////////// TEXTURES ////////////////////////////////////
	Texture2D texture;

///////////////////////////////// SHADERS /////////////////////////////////////
	ShaderProgram shader;

//////////////////////////////// PROPERTIES ///////////////////////////////////

	// Scene lights
	PointLight pointLight;
	DirectionalLight directionalLight;

	// A robot arm model
	RobotArm robotArm;

public:
	ParticleSystem ps;

	/** Modeler calls this to get our particle system. */
	ParticleSystem* getParticleSystem() { return NULL; }

	/** Construct the scene */
	Scene() :
	    // You have to call the parent class's constructor, to provide a
	    // name for the model.
	    Model("Scene"),

		// Construct textures and shaders. 
		// They won't be loaded until the model is drawn for the first time.
		texture("checkers.png"),
		shader("shader.vert", "shader.frag", NULL),

		// Call the constructors for the lights
		pointLight("Point Light", GL_LIGHT1, /**direction part**/ -5, 5, 5, /**diffuse part**/ 1.0, 0.5, 0.5, 
		/**specular part**/ 1.0, 0.5, 0.5, /**ambient part**/ .2f, 0.1, 0.1 /**attenuation part**/, 0.4, 0.7, 0),
		directionalLight("Directional Light", GL_LIGHT0, /**direction part**/ 5, 5, 5, /**diffuse part**/ 0.0f, 0.5, 0.5f, 
		/**specular part**/ 0.0f, 0.5f, 0.5f )

		// Now, call the constructors for each Property:
	{
		// If you have child Models, like the MobileLight model from model.h,
		// you can add their property groups, and they will appear in the list
		// in the top left corner of Modeler, under this model's entry:
		properties.add(pointLight.getProperties())
				  .add(directionalLight.getProperties());
		properties.add(robotArm.getProperties());

		// Finally, add all the properties to this model's PropertyGroup.
	}

	/**
	 * Modeler calls this method once an OpenGL context becomes available,
	 * (which happens before the first time draw() is called), and whenever
	 * you click the "Reload Textures and Shaders" menu option.
	 *
	 * Make sure you call the load() methods of each of your textures and
	 * shaders here.
	 */
	void load() {
		texture.load();
		shader.load();
	}

	/**
	 * Modeler calls this method many times a second when the Animate
	 * checkbox is checked.
	 */
	void tick() {
		// You can put code here to increment animation counters for
		// extra credit.
	}


	/** Draw the scene. */
	void draw() {
		// The lights must be drawn FIRST, so the other scene elements
		// can get lit!
		pointLight.draw();
		directionalLight.draw();


		// Draw robot arm
		robotArm.draw();
	}
};

/**
 * The program starts here.
 */
int main()
{
	// Tell the FLTK library to let us use OpenGL
	Fl::gl_visual(FL_RGB);

	init_load_curve_file();

	// Instantiate Modeler
	ModelerUserInterface ui;

	// Give Modeler your scene.
	// Modeler will free this pointer when it exits.
	ui.setModel(new Scene());


	// Run the application
    return ui.run();
}