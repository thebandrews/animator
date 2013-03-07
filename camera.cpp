#include <GL/glew.h>

#include "camera.h"
#include "modelerdraw.h"
#include "trackball.h"
#include "cameras.h"

#pragma warning(push)
#pragma warning(disable : 4244)

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502f
#endif 

#define CAMERA_HALF_WIDTH (0.2f)

const float kMouseRotationSensitivity		= 1.0f/90.0f;
const float kMouseTranslationXSensitivity	= 0.03f;
const float kMouseTranslationYSensitivity	= 0.03f;
const float kMouseZoomSensitivity			= 0.08f;

/**
 * Constructs a new Camera with the given viewport dimensions.
 * If you want to make changes to the camera's initial state,
 * you could do it here.
 */
Camera::Camera(int viewportWidth, int viewportHeight) :
	mCurrentMouseAction(kActionNone),
	m_vLookAt(0,0,0),
	m_fDolly(20),
	m_fFOV(40),
	m_iViewportWidth(viewportWidth),
	m_iViewportHeight(viewportHeight),
	m_fNearClip(1),
	m_fFarClip(300),
	m_fHalfHeight(CAMERA_HALF_WIDTH*viewportHeight/viewportWidth),
	m_fHalfWidth(CAMERA_HALF_WIDTH)
	, target(NULL),
	syncWithTarget(false),
	ignoreSliders(false)
{
	float axis[3] = {1, 0, 0};
	axis_to_quat(axis, -30 * M_PI / 180, m_fQuat);
	updateTransform();
}

void Camera::setLookAt( const Vec3f &lookAt )
{
	m_vLookAt = lookAt;
	m_bDirtyTransform = true;
}

const Vec3f& Camera::getLookAt() const
{
	return m_vLookAt;
}

void Camera::setQuat( const float* quat )
{
	memcpy(m_fQuat, quat, sizeof(float) * 4);
	m_bDirtyTransform = true;
}

const float* Camera::getQuat() const
{
	return m_fQuat;
}


void Camera::setDolly( const float dolly )
{
	// Keep the look-at point in front of the near clipping plane
	if ( dolly < m_fNearClip )
	{
		m_fDolly = m_fNearClip;
	}
	else
	{
		m_fDolly = dolly;
	}
	m_bDirtyTransform = true;
}
const float Camera::getDolly() const
{
	return m_fDolly;
}

void Camera::clickMouse( MouseAction_t action, int x, int y )
{
	mCurrentMouseAction = action;
	mLastMousePosition[0] = x;
	mLastMousePosition[1] = y;
}

void Camera::updateTransform()
{
	// Store the rotation matrix here.
	float matRot[4][4];

	// Determine rotation matrix from quaternion.
	build_rotmatrix(matRot, m_fQuat);

	// Normal camera direction
	Vec3f vDirection(0,0,m_fDolly);
	Vec3f vUpVector(0,1,0);

	// Apply the rotation matrix to determine the camera's new direction.
	Vec3f vNewDirection(
		matRot[0][0] * vDirection[0] + matRot[0][1] * vDirection[1] + matRot[0][2] * vDirection[2],
		matRot[1][0] * vDirection[0] + matRot[1][1] * vDirection[1] + matRot[1][2] * vDirection[2],
		matRot[2][0] * vDirection[0] + matRot[2][1] * vDirection[1] + matRot[2][2] * vDirection[2]
	);

	// Apply the rotation matrix to determine the camera's new up-vector.
	Vec3f vNewUpVector(
		matRot[0][0] * vUpVector[0] + matRot[0][1] * vUpVector[1] + matRot[0][2] * vUpVector[2],
		matRot[1][0] * vUpVector[0] + matRot[1][1] * vUpVector[1] + matRot[1][2] * vUpVector[2],
		matRot[2][0] * vUpVector[0] + matRot[2][1] * vUpVector[1] + matRot[2][2] * vUpVector[2]
	);

	// Update camera.
	m_vPosition = m_vLookAt + vNewDirection;
	m_vUpVector = vNewUpVector;

	// Not dirty anymore
	m_bDirtyTransform = false;
}

void Camera::dragMouse( int x, int y )
{
	Vec3f mouseDelta   = Vec3f(x,y,0.0f) - mLastMousePosition;

	switch(mCurrentMouseAction)
	{
	case kActionTranslate:
		{
			// Determine mouse movement
			double xTrack =  -mouseDelta[0] * kMouseTranslationXSensitivity;
			double yTrack =  mouseDelta[1] * kMouseTranslationYSensitivity;

			// Recalculate stuff if needed
			if (m_bDirtyTransform)
			{
				updateTransform();
			}

			// Determine directions of motion in scene space
			Vec3f direction = m_vPosition - m_vLookAt;
			Vec3f transXAxis = m_vUpVector ^ direction;
			transXAxis /= sqrt((transXAxis*transXAxis));
			Vec3f transYAxis = direction ^ transXAxis;
			transYAxis /= sqrt((transYAxis*transYAxis));

			// Move the camera's look-at point
			setLookAt(getLookAt() + transXAxis*xTrack + transYAxis*yTrack);
			
			break;
		}
	case kActionRotate:
		{
			// Store the rotation in this quarternion
			float quat[4];

			// Get the mouse coordinates in a range between -1.0 and 1.0
			float viewHalfWidth = (m_iViewportWidth / 2.f), viewHalfHeight = (m_iViewportHeight / 2.f);
			float oldX = mLastMousePosition[0] * 2.f / (m_iViewportWidth - 1) - 1.f,
				  oldY = mLastMousePosition[1] * 2.f / (m_iViewportHeight - 1) - 1.f,
				  newX = x * 2.f / (m_iViewportWidth - 1) - 1.f,
				  newY = y * 2.f / (m_iViewportHeight - 1) - 1.f;

			// Get the quaternion to rotate around, from the trackball code.
			trackball( quat, oldX, -oldY, newX, -newY );

			// Add the new quaternion to the current one.
			float oldQuat[4];
			memcpy(oldQuat, m_fQuat, sizeof(float) * 4);
			//add_quats(oldQuat, quat, m_fQuat);
			add_quats(quat, oldQuat, m_fQuat);

			// Update the transform parameters.
			updateTransform();

			break;
		}
	case kActionZoom:
		{
			// Determine dolly movement.
			float fDollyDelta = -mouseDelta[1] * kMouseZoomSensitivity;

			// Add to dolly
			setDolly(getDolly() + fDollyDelta);

			updateTransform();

			break;
		}
	case kActionTwist:
		{
			break;
		}
	default:
		break;
	}

	mLastMousePosition = Vec3f(x,y,0.0f);

	// Put updated values into the target camera.
	if (target) {
		ignoreSliders = true;
		target->fromVectors(m_vPosition, m_vLookAt, m_vUpVector, m_fFOV,
			(float)m_iViewportWidth / m_iViewportHeight);
		ignoreSliders = false;
	}
}

void Camera::releaseMouse( int x, int y )
{
	mCurrentMouseAction = kActionNone;
}

void Camera::applyViewingTransform()
{
	// Synchronize with the target camera if needed.
	if (syncWithTarget && target) {
		Vec3f pos, look, y;
		target->toVectors(pos, look, y, m_fFOV);
		m_vPosition = pos;
		m_vLookAt = look;
		m_vUpVector = y;

		// Calculate direction and dolly.
		Vec3f z = pos - look;
		m_fDolly = z.length();
		z.normalize();

		// Calculate rotation matrix.
		Vec3f x = y ^ z;
		float rotMat[12] = {
			x[0], x[1], x[2], 0,
			y[0], y[1], y[2], 0,
			z[0], z[1], z[2], 0
		};

		// Determine quaternion from the rotation matrix.
		// We don't need to update the transform because it's already updated.
		Vec4f q = mat2quat(rotMat);
		for (int i = 0; i < 4; i++) { m_fQuat[i] = q[i]; }

		syncWithTarget = false;
	} else
	if (m_bDirtyTransform) {
		updateTransform();
	}

	ModelerDrawState *mds = ModelerDrawState::Instance();

	/* If we're outputting the scene to a raytracer file,
	 * output the camera info to the file.
	 */
	if(mds->m_rayFile)
	{
		fprintf( mds->m_rayFile, "camera {\n\tposition = (%f, %f, %f);\n\tlook_at = (%f, %f, %f);\n\taspectratio = %f\n\tfov = %f; }\n\n",
			m_vPosition[0], m_vPosition[1], m_vPosition[2],
			m_vLookAt[0], m_vLookAt[1], m_vLookAt[2], ((double) m_iViewportWidth) / m_iViewportHeight, m_fFOV);
		return;
	}

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

#define PMP557

#ifndef PMP557
	// Place the camera at v_position, aim the camera at
	// m_vLookAt, and twist the camera such that m_vUpVector is up
	// YOU WILL NEED TO REPLACE THIS FUNCTION CALL WITH YOUR OWN CODE!
	gluLookAt(	m_vPosition[0], m_vPosition[1], m_vPosition[2],
				m_vLookAt[0],   m_vLookAt[1],   m_vLookAt[2],
				m_vUpVector[0], m_vUpVector[1], m_vUpVector[2]);
#else
	gluLookAt(	m_vPosition[0], m_vPosition[1], m_vPosition[2],
				m_vLookAt[0],   m_vLookAt[1],   m_vLookAt[2],
				m_vUpVector[0], m_vUpVector[1], m_vUpVector[2]);
#endif

}

void Camera::applyProjectionTransform()
{
	// Calculate half the width using the aspect ratio of the viewport
	//float fHalfWidth = m_iViewportWidth * m_fHalfHeight / m_iViewportHeight;

	// Modify the projection matrix.
	glMatrixMode( GL_PROJECTION );

	// Create the viewing volume (see OpenGL documentation for how this works)
	glLoadIdentity();
	// can also use glFrustum( -m_fHalfWidth, m_fHalfWidth, -m_fHalfHeight, m_fHalfHeight, m_fNearClip, m_fFarClip );
	gluPerspective(m_fFOV, ((double) m_iViewportWidth) / m_iViewportHeight,
		m_fNearClip, m_fFarClip);
}

void Camera::setViewport()
{
	glViewport( 0, 0, m_iViewportWidth, m_iViewportHeight );
}

void Camera::resizeViewport( int width, int height )
{
	// Exit if the parameters are invalid
	if ( width <= 0 || height <= 0 )
	{
		return;
	}

	// Adjust the camera width so the aspect ratio is maintained.
	m_fHalfWidth = width * m_fHalfHeight / height;

	// Store the new values.
	m_iViewportWidth = width;
	m_iViewportHeight = height;

	// Set the viewport
	setViewport();
}

void Camera::setTarget(ICamera* _target) {
	// Save the target pointer
	target = _target;

	if (!target) { return; }

	// Copy the camera's view into this camera on next draw.
	syncWithTarget = true;

	// Update this camera when the target camera changes
	target->listen(requestSync, this);
}

void Camera::requestSync(SignalDispatcher* d, void* v, void* a) {
	if (!((Camera*)v)->ignoreSliders) {
		((Camera*)v)->syncWithTarget = true;
	}
}

ICamera* Camera::getTarget() { return target; }

#pragma warning(pop)
