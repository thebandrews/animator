// Vertex shader for per-fragment Phong interpolation and shading.


// These variables will be computed per vertex but will vary across
// each triangle, to be used by the fragment shader for lighting
// calculations.
varying vec3 N;
varying vec3 v;

void main()
{
	// The eye is sitting at the origin in eye space.  After
	// transforming a vertex into eye space with the modelview
	// matrix, the vector from the vertex to the eye is the
	// viewing direction.  We will pass along the transformed
	// vertex location to be interpolated across the triangle and
	// then used to compute viewing and lighting directions.
	v = vec3(gl_ModelViewMatrix * gl_Vertex);

	// Need to transform the normal into eye space.
	N = normalize(gl_NormalMatrix * gl_Normal);

	// Always have to transform vertex positions so they end
	// up in the right place on the screen.
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

