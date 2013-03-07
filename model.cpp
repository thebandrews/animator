#include "model.h"
#include <algorithm>
#include "modelerdraw.h"

using namespace std;

///////////// class Model ///////////////
Model::Model(const char* name) : properties(name) {}
GroupProperty* Model::getProperties() { return &properties; }
void Model::draw() {}
void Model::tick() {}
void Model::load() {}
ICamera* Model::getCamera() { return NULL; }
ParticleSystem* Model::getParticleSystem() { return NULL; };

///////////// class Light ///////////////
Light::Light(const char* name, GLuint lightNumber,
						 float x, float y, float z,
						 float dr, float dg, float db,
						 float sr, float sg, float sb,
						 float ar, float ag, float ab) :
	Model(name),
    lightX("Light X", -10, 10, x, .1f),
	lightY("Light Y", -10, 10, y, .1f),
	lightZ("Light Z", -10, 10, z, .1f),
	diffuse("Diffuse", dr, dg, db),
	specular("Specular", sr, sg, sb),
	ambient("Ambient",  ar, ag, ab),
	isOn("This light is on", true)
	{

	properties.add(&isOn)
			  .add(&diffuse)
			  .add(&specular)
			  .add(&ambient)
			  .add(&lightX)
			  .add(&lightY)
			  .add(&lightZ);
	
	properties.setCollapsed(true);

	this->lightNumber = lightNumber;
}

void Light::draw(bool directional) {
	if (!isOn.getValue()) {
		// Set values to 0
		const float zeroes[4] = {0,0,0,0};
		glLightfv( lightNumber, GL_DIFFUSE, zeroes );
		glLightfv( lightNumber, GL_SPECULAR, zeroes );
		glLightfv( lightNumber, GL_AMBIENT, zeroes );
		return;
	}

	// Enable OpenGL lighting
	// TODO: check to see if it's already on
	glEnable( GL_LIGHTING );

	// Enable the light we're using
	// TODO: check to see if it's already on
	glEnable(lightNumber);

	// Create array containing light position
	GLfloat lightPositionArray[4] = {
		lightX.getValue(),
		lightY.getValue(),
		lightZ.getValue(),
		directional ? 0.f : 1.f
	};

	// Return if the light is directional but has no direction.
	if (directional && lightPositionArray[0] == 0
		&& lightPositionArray[1] == 0
		&& lightPositionArray[2] == 0) return;

	// Create array containing light diffuse color
	GLfloat lightDiffuseArray[4] = {
		diffuse.getRed(),
		diffuse.getGreen(),
		diffuse.getBlue(),
		1
	};

	// Create array for light's specular color
	GLfloat lightSpecularArray[4] = {
		specular.getRed(),
		specular.getGreen(),
		specular.getBlue(),
		1
	};

	// Create array for light's ambient color
	GLfloat lightAmbientArray[4] = {
		ambient.getRed(),
		ambient.getGreen(),
		ambient.getBlue(),
		1
	};

	// Send light to OpenGL
	glLightfv( lightNumber, GL_POSITION, lightPositionArray );
	glLightfv( lightNumber, GL_DIFFUSE, lightDiffuseArray );
	glLightfv( lightNumber, GL_SPECULAR, lightSpecularArray );
	glLightfv( lightNumber, GL_AMBIENT, lightAmbientArray );

	// Draw a marker for the light if allowed
	if (ModelerDrawState::Instance()->showMarkers) {
		const float lightRadius = .5f;
		const float directionalOrbit = 5.f;
		
		glPushMatrix();
			glTranslatef(lightPositionArray[0],
						 lightPositionArray[1],
						 lightPositionArray[2]);
			glDisable(GL_LIGHTING);
			glDisable(GL_TEXTURE_2D);
			glColor3f(1, 1, 1);
			glBegin(GL_LINES);
				glVertex3f(-lightRadius, 0, 0);
				glVertex3f(lightRadius, 0, 0);
				glVertex3f(0, -lightRadius, 0);
				glVertex3f(0, lightRadius, 0);
				glVertex3f(0, 0, -lightRadius);
				glVertex3f(0, 0, lightRadius);
			glEnd();
			glEnable(GL_LIGHTING);
		glPopMatrix();
	}
}

PointLight::PointLight(const char* name, GLuint lightNumber,
				float x, float y, float z,
				float dr, float dg, float db,
				float sr, float sg, float sb,
				float ar, float ag, float ab,
				float attA, float attB, float attC) :
	Light(name, lightNumber, x, y, z, dr, dg, db, sr, sg, sb, ar, ag, ab),
	attenA("Quadratic Attenuation ^ 5", 0, 1, attA, .05f),
	attenB("Linear Attenuation ^ 5", 0, 1, attB, .05f),
	attenC("Constant Attenuation ^ 5", 0, 1, attC, .05f)
{
	properties.add(&attenA).add(&attenB).add(&attenC);
}

void PointLight::draw() {
	// Set attenuation
	glLightf(lightNumber, GL_CONSTANT_ATTENUATION, pow(attenC.getValue(), 5));
	glLightf(lightNumber, GL_LINEAR_ATTENUATION, pow(attenB.getValue(), 5));
	glLightf(lightNumber, GL_QUADRATIC_ATTENUATION, pow(attenA.getValue(), 5));

	Light::draw(false);
}

/**
 * A directional light.
 */
DirectionalLight::DirectionalLight(const char* name, GLuint lightNumber,
				float x, float y, float z,
				float dr, float dg, float db,
				float sr, float sg, float sb) :
	Light(name, lightNumber, x, y, z, dr, dg, db, sr, sg, sb)
{
}

void DirectionalLight::draw() {
	Light::draw(true);
}