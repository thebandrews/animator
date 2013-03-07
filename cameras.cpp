#include "cameras.h"
#include "modelerui.h"

// We can't declare pure virtual destructors, so ICamera needs this:
ICamera::~ICamera() {}

SimpleCameraModel::SimpleCameraModel(const char* name, Vec3f& _lookAt,
									 float _azimuth, float _elevation,
									 float _dolly, float _twist, float _fov) :
	Model(name),
	azimuth("Azimuth", -180, 180, _azimuth, 1),
	elevation("Elevation", -90, 90, _elevation, 1),
	dolly("Dolly", 0, 100, _dolly, .1f),
	twist("Twist", -180, 180, _twist, 1),
	lookAtX("LookAt X", -10, 10, _lookAt[0], .1f),
	lookAtY("LookAt Y", -10, 10, _lookAt[1], .1f),
	lookAtZ("LookAt Z", -10, 10, _lookAt[2], .1f),
	fov("Field of View", 10, 170, _fov, 1.f) {

	properties.add(&azimuth).add(&elevation).add(&dolly).add(&twist)
		.add(&lookAtX).add(&lookAtY).add(&lookAtZ).add(&fov);

	azimuth.listen(onChange, this);
	elevation.listen(onChange, this);
	dolly.listen(onChange, this);
	twist.listen(onChange, this);
	lookAtX.listen(onChange, this);
	lookAtY.listen(onChange, this);
	lookAtZ.listen(onChange, this);
	fov.listen(onChange, this);
}

void SimpleCameraModel::draw() {
	if (!ModelerDrawState::Instance()->showMarkers) { return; }

	Vec3f position, lookAt, up;
	float fov;
	toVectors(position, lookAt, up, fov);
	Vec3f direction = lookAt - position;
	direction.normalize();
	Vec3f side = direction ^ up;
	side.normalize();
	float h = tan(fov * M_PI / 360);
	side *= h; 
	up.normalize();
	up *= h * aspectRatio;

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glColor3i(1,0,0);
	glLineWidth(2);

	Vec3f topLeft = position + direction - side + up,
		topRight = position + direction + side + up,
		bottomLeft = position + direction - side - up,
		bottomRight = position + direction + side - up;
	glBegin(GL_LINES);
		position.glVertex(); topLeft.glVertex();
		position.glVertex(); topRight.glVertex();
		position.glVertex(); bottomLeft.glVertex();
		position.glVertex(); bottomRight.glVertex();
	glEnd();

	glBegin(GL_LINE_LOOP);
		topLeft.glVertex();
		topRight.glVertex();
		bottomRight.glVertex();
		bottomLeft.glVertex();
	glEnd();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}

void SimpleCameraModel::plotKeyframe(float t) {
	// Iterate over list of model properties
	PropertyList* props = properties.getProperties();
	for (PropertyList::iterator iter = props->begin();
		iter != props->end();
		iter++) {

		// Cast property to a range property
		RangeProperty* rp = dynamic_cast<RangeProperty*>(*iter);
		if (!rp) { continue; }

		// Get the curve, and add a control point
		Curve* pCurve = ModelerUserInterface::getInstance()->
			graph->curve(rp->getCurveIndex());
        pCurve->addControlPoint( Animator::Point(t, rp->getValue()) );
	}

	// Redraw the curves
	ModelerUserInterface::getInstance()->graph->redraw();
}

void SimpleCameraModel::clearKeyframe(float t) {
	// Iterate over list of model properties
	PropertyList* props = properties.getProperties();
	for (PropertyList::iterator iter = props->begin();
		iter != props->end();
		iter++) {

		// Attempt to cast property to a range property
		RangeProperty* rp = dynamic_cast<RangeProperty*>(*iter);
		if (!rp) { continue; }

		// Get the curve
		Curve* pCurve = ModelerUserInterface::getInstance()->
			graph->curve(rp->getCurveIndex());

		// Remove all control points in a small range around the current time.
		pCurve->removeControlPointsInRange(
			t-REMOVE_KEYFRAME_RANGE/2, t+REMOVE_KEYFRAME_RANGE/2 );
	}

	// Redraw the curves
	ModelerUserInterface::getInstance()->graph->redraw();
}

void SimpleCameraModel::onChange(SignalDispatcher* d, void* v, void* a) {
	((SimpleCameraModel*)v)->signal();
}

void SimpleCameraModel::fromVectors(Vec3f& position, Vec3f& lookAt, Vec3f& up, float _fov, float _aspectRatio) {
	const float EPSILON = .0000001;
	lookAtX.setValue(lookAt[0]);
	lookAtY.setValue(lookAt[1]);
	lookAtZ.setValue(lookAt[2]);
	fov.setValue(_fov);
	aspectRatio = _aspectRatio;

	// Get direction and dolly
	Vec3f direction = lookAt - position;
	dolly.setValue((float)direction.length());
	direction.normalize();

	// We're going to assume the world's up-vector is (0, 1, 0) for this camera.
	// Thus, the given up-vector will be used solely for determining twist.

	// Convert direction vector to angles
	Vec3f upComponent = (direction * Vec3f(0.0, 1.0, 0.0)) * Vec3f(0.0, 1.0, 0.0);
	float outLength = sqrt(direction[0] * direction[0] + direction[2] * direction[2]);
	if (outLength < EPSILON) {
		// The camera is looking straight up or down, so we can't distinguish the
		// azimuth from the twist.  So, we'll just assume that there is no twist,
		// and set the azimuth using the given up-vector.
		twist.setValue(0);
		azimuth.setValue((float)atan2(-up[2], -up[0]) * 180 / M_PI);
	} else {
		azimuth.setValue((float)atan2(direction[0], -direction[2]) * 180 / M_PI);

		// Determine camera out-vector and up-vector with no twist
		Vec3f out = direction ^ Vec3f(0, 1, 0);
		Vec3f realUp = out ^ direction;

		// Set twist based on projection of the given up-vector onto the up-vector
		// and out-vector with no twist.
		twist.setValue((float)atan2(-out * up, realUp * up) * 180 / M_PI);
	}
	elevation.setValue((float)atan2(-direction[1], outLength) * 180 / M_PI);
}

void SimpleCameraModel::toVectors(Vec3f& position, Vec3f& lookAt, Vec3f& up, float& _fov) {
	// Set FOV
	_fov = fov.getValue();

	// Set look-at vector
	lookAt = Vec3f(lookAtX.getValue(), lookAtY.getValue(), lookAtZ.getValue());

	// Compute direction vector
	float elevRadians = (float)elevation.getValue() * M_PI / 180;
	float elevUp = sin(elevRadians);
	float elevOut = cos(elevRadians);
	float aziRadians = (float)azimuth.getValue() * M_PI / 180;
	float aziZ = cos(aziRadians);
	float aziX = -sin(aziRadians);
	Vec3f direction(aziX * elevOut, elevUp, aziZ * elevOut);

	// Calculate position
	position = lookAt + direction * dolly.getValue();

	// Calculate up-vector.
	up = Vec3f(-aziX * elevUp, elevOut, -aziZ * elevUp);

	float twistRadians = (float)twist.getValue() * M_PI / 180;
	float twistUp = cos(twistRadians), twistOut = sin(twistRadians);

	// Calculate out-vector, which corresponds to the +x axis in camera space.
	Vec3f out = direction ^ up;

	// Rotate the up-vector around the direction vector.
	up = up * twistUp + out * twistOut;
}