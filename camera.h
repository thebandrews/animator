/******************************************************
 * The camera class represents a camera that is
 * a given distance away from a point, focusing on
 * that point.
 *
 * Authors:
 *  - Jeffrey Booth (boothjmx@cs)
 *  - ehsu
 ******************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include "vec.h"
#include "mat.h"
#include "trackball.h"
#include "model.h"

typedef enum { kActionNone, kActionTranslate, kActionRotate, kActionZoom, kActionTwist,} MouseAction_t;

class Camera {
    
protected:
	/** The point the camera is looking at, in scene space. */
    Vec3f       m_vLookAt;

	/**
	 * Quaternion defining the camera's orientation.
	 */
	float       m_fQuat[4];

	/**
	 * A normalized vector indicating the direction that
	 * corresponds to "up", in scene space.
	 * TODO: describe calculation.
	 */
	Vec3f       m_vUpVector;

	/**
	 * Distance from the camera to the look-at point.
	 */
	float       m_fDolly;

	/**
	 * Camera field of view angle in the y-direction.
	 */
	float       m_fFOV;

	/**
	 * The camera's position in scene space.
	 * TODO: describe calculation.
	 */
	Vec3f       m_vPosition;

	/** Viewport width, in pixels. */
	int			m_iViewportWidth;

	/** Viewport height, in pixels. */
	int         m_iViewportHeight;

	/**
	 * Distance from the center of the camera view to the top
	 * of the camera screen, in scene space.  This vector is
	 * used to determine the camera's vertical field of vision,
	 * and is half the height of the near clipping plane.
	 */
	float		m_fHalfHeight;

	/** Same as m_fHalfHeight, but for the camera width in scene space. */
	float       m_fHalfWidth;

	/**
	 * Distance from the camera to the near clipping plane of
	 * its viewing volume / frustum.
	 */
	float		m_fNearClip;

	/**
	 * Distance from the camera to the far clipping plane of
	 * its viewing volume / frustum.
	 */
	float		m_fFarClip;

	/**
	 * Current mouse action
	 * TODO: remove
	 */
	MouseAction_t mCurrentMouseAction;

	/**
	 * Last mouse position
	 */
	Vec3f       mLastMousePosition;

	/**
	 * True if the camera viewing parameters need updating.
	 */
	bool        m_bDirtyTransform;

	/**
	 * The ICamera object that this camera should sync with.  This object
	 * is provided either by models or by ModelerUserInterface.
	 */
	ICamera* target;
	
public:

	/**
	 * True if we're ignoring sliders.  This flag is used to prevent the
	 * camera from responding to its properties' events when a slider has
	 * moved as a result of the user dragging the mouse in the model view area.
	 */
	bool ignoreSliders;
    
    //---[ Constructors ]----------------------------------
    
    /**
	 * Creates a Camera looking at (0,0,0) from the point (0,0,20),
	 * looking down the negative z-axis with the up-vector
	 * pointing along the positive y-axis.
	 */
    Camera(int viewportWidth, int viewportHeight);
    
    //---[ Settings ]--------------------------------------
    
    void setLookAt( const Vec3f &lookAt );
    const Vec3f& getLookAt() const;

	void setQuat( const float* quat );
	const float* getQuat() const;

	void setDolly( const float dolly );
	const float getDolly() const;
    
    //---[ Interactive Adjustment ]------------------------
    // these should be used from a mouse event handling routine that calls
    // the startX method on a mouse down, updateX on mouse move and finally
    // endX on mouse up.
    //-----------------------------------------------------
    void clickMouse( MouseAction_t action, int x, int y );
    void dragMouse( int x, int y );
    void releaseMouse( int x, int y );
    
    //---[ Viewing Transform ]--------------------------------
	/**
	 * Applies the camera viewing transform to the modelview matrix.
	 */
    void applyViewingTransform();

	/**
	 * Applies the camera projection transform to the projection matrix.
	 */
	void applyProjectionTransform();

	/**
	 * Resizes the camera's viewport to the indicated width and
	 * height in pixels.
	 */
	void resizeViewport( int width, int height );

	/**
	 * Sets up the viewport.
	 */
	void setViewport();

	/**
	 * Update the transform variables like m_vPosition and
	 * m_fQuat.
	 */
	void updateTransform();

	/** Set the ICamera object to control */
	void setTarget(ICamera* _target);

	/** Get the current camera object. */
	ICamera* getTarget();

	/**
	 * True when this camera should synchronize with its target on the next
	 * draw.
	 */
	boolean syncWithTarget;

	/** Called when the target camera object is modified. */
	static void requestSync(SignalDispatcher* d, void* v, void* a);
};

#endif