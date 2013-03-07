#ifndef MODEL_H
#define MODEL_H

#include <cmath>
#include "vec.h"
#include "properties.h"

/** Forward-declare ICamera because it's used in Model yet is a Model. */
class ICamera;

/**
 * Forward-declare ParticleSystem because it's used in Model yet it's a
 * Model.
 */
class ParticleSystem;

/**
 * You will extend this class to create your model.
 */
class Model {
protected:
	/**
	 * A GroupProperty containing your model's name and a list of its
	 * properties.  Modeler will obtain this object using the getProperties()
	 * method, so it can link controls to your properties.
	 */
	GroupProperty properties;

public:
	Model(const char* name);

	/**
	 * This method returns the model's list of properties.
	 */
	GroupProperty* getProperties();

	/**
	 * Override this method to draw your model to the current OpenGL context.
	 */
	virtual void draw();

	/**
	 * (Optional) Override this method to update your animation counters.
	 */
	virtual void tick();

	/**
	 * Override this method to load your textures, shaders, and anything else
	 * that requires an OpenGL context.
	 */
	virtual void load();

	/**
	 * Override this method to return the camera you want to look through.
	 */
	virtual ICamera* getCamera();

	/**
	 * Override this method to return the particle system you want to control.
	 */
	virtual ParticleSystem* getParticleSystem();
};

/**
 * This class is abstract.  Don't mess with it.
 */
class Light : public Model {
protected:
	GLuint lightNumber;

public:
	RGBProperty diffuse, specular, ambient;
	BooleanProperty isOn;
	RangeProperty lightX, lightY, lightZ;

	Light(const char* name, GLuint lightNumber,
				float x = 0, float y = 0, float z = 0,
				float dr = 1, float dg = 1, float db = 1,
				float sr = 1, float sg = 1, float sb = 1,
				float ar = .1, float ag = .1, float ab = .1);

	void draw(bool directional);
};

/**
 * A point light.
 */
class PointLight : public Light {
public:
	RangeProperty attenA, attenB, attenC;

	PointLight(const char* name, GLuint lightNumber,
				float x = 0, float y = 0, float z = 0,
				float dr = 1, float dg = 1, float db = 1,
				float sr = 1, float sg = 1, float sb = 1,
				float ar = .1, float ag = .1, float ab = .1,
				float attA = .5, float attB = .7, float attC = 0);

	void draw();
};

/**
 * A directional light.
 */
class DirectionalLight : public Light {
public:
	DirectionalLight(const char* name, GLuint lightNumber,
				float x = 0, float y = 0, float z = 0,
				float dr = 1, float dg = 1, float db = 1,
				float sr = 1, float sg = 1, float sb = 1);

	void draw();
};

#endif // MODEL_H