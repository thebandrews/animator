#ifndef CAMERAS_H
#define CAMERAS_H
/**
 * This file contains the ICamera interface, and a camera that implements it.
 *
 * Do you want to have MULTIPLE cameras in your model?  Just add multiple
 * SimpleCameraModels to your model, then override the getCamera() method to
 * return a SimpleCameraModel.
 *
 * If you're feeling really ambitious, implement a new kind of camera that
 * implements interface ICamera.
 */

#include "model.h"

/**
 * Classes implementing this interface can be controlled like cameras,
 * meaning that you can manipulate them by dragging your mouse in the
 * model view window.
 */
class ICamera : public SignalDispatcher {
public:
	/**
	 * Set this camera using the given vectors.
	 */
	virtual void fromVectors(Vec3f& position, Vec3f& lookAt, Vec3f& up,
		float _fov, float _aspectRatio) = 0;

	/**
	 * Convert this camera to vectors.
	 */
	virtual void toVectors(Vec3f& position, Vec3f& lookAt, Vec3f& up,
		float& fov) = 0;

	/**
	 * Have the camera plot its position at the given time.
	 */
	virtual void plotKeyframe(float t) = 0;

	/**
	 * Remove the current keyframe
	 */
	virtual void clearKeyframe(float t) = 0;

	/** Make sure the subclass's destructor is called */
	virtual ~ICamera();
};

/**
 * A basic camera, with a constrained up-vector.  TODO: unconstrain this vector!
 */
class SimpleCameraModel : public Model, public ICamera {
protected:
	RangeProperty azimuth, elevation, dolly, twist,
		lookAtX, lookAtY, lookAtZ, fov;
	float aspectRatio;

public:
	SimpleCameraModel(const char* name,
		Vec3f& lookAt = Vec3f(0, 0, 0), float azimuth = 0,
		float elevation = 30, float dolly = 20, float twist = 0,
		float fov = 40);

	void fromVectors(Vec3f& position, Vec3f& lookAt, Vec3f& up, float _fov, float _aspectRatio);
	void toVectors(Vec3f& position, Vec3f& lookAt, Vec3f& up, float& _fov);
	static void onChange(SignalDispatcher* d, void* v, void* a);
	void plotKeyframe(float t);
	void clearKeyframe(float t);
	void draw();
};

#endif // CAMERAS_H