#include "modelerdraw.h"
#include <FL/gl.h>
#include <GL/glu.h>
#include <cstdio>
#include <math.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

// Helper functions from the red book so we can print text on the
// screen.
GLubyte space[] =
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
GLubyte letters[][13] = {
    {0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
    {0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
    {0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
    {0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc},
    {0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
    {0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff},
    {0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
    {0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
    {0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e},
    {0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
    {0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3},
    {0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
    {0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3},
    {0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3},
    {0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
    {0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
    {0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c},
    {0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
    {0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e},
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff},
    {0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
    {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
    {0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
    {0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
    {0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
    {0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff}
};

GLuint fontOffset;

void makeRasterFont(void)
{
   GLuint i, j;
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei( GL_UNPACK_ROW_LENGTH, 8);

   fontOffset = glGenLists (128);
   for (i = 0,j = 'A'; i < 26; i++,j++) {
      glNewList(fontOffset + j, GL_COMPILE);
      glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, letters[i]);
      glEndList();
   }
   glNewList(fontOffset + ' ', GL_COMPILE);
   glBitmap(8, 13, 0.0, 2.0, 10.0, 0.0, space);
   glEndList();
}

void printString(char *s)
{
   static bool haveInitialized = false;
   if( !haveInitialized ) {
	   makeRasterFont();
	   haveInitialized = true;
   }
   glPushAttrib (GL_LIST_BIT);
   glListBase(fontOffset);
   glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *) s);
   glPopAttrib ();
}

// ********************************************************
// Support functions from previous version of modeler
// ********************************************************
static void _dump_current_modelview( void )
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    if (mds->m_rayFile == NULL)
    {
        fprintf(stderr, "No .ray file opened for writing, bailing out.\n");
        exit(-1);
    }
    
    GLdouble mv[16];
    glGetDoublev( GL_MODELVIEW_MATRIX, mv );
    fprintf( mds->m_rayFile, 
        "transform(\n\t(%f, %f, %f, %f),\n\t(%f, %f, %f, %f),\n\t(%f, %f, %f, %f),\n\t(%f, %f, %f, %f),\n",
        mv[0], mv[4], mv[8], mv[12],
        mv[1], mv[5], mv[9], mv[13],
        mv[2], mv[6], mv[10], mv[14],
        mv[3], mv[7], mv[11], mv[15] );
}

static void _dump_current_material( void )
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    if (mds->m_rayFile == NULL)
    {
        fprintf(stderr, "No .ray file opened for writing, bailing out.\n");
        exit(-1);
    }
    
    fprintf( mds->m_rayFile, 
        "material={\n\tdiffuse = (%f, %f, %f);\n\tambient = (%f, %f, %f);\n\tspecular = (%f, %f, %f);\n\tshininess = %f; }\n",
        mds->m_diffuseColor[0], mds->m_diffuseColor[1], mds->m_diffuseColor[2], 
        mds->m_ambientColor[0], mds->m_ambientColor[1], mds->m_ambientColor[2],
		mds->m_specularColor[0], mds->m_specularColor[1], mds->m_specularColor[2],
		mds->m_shininess);
}

// ****************************************************************************

// Initially assign singleton instance to NULL
ModelerDrawState* ModelerDrawState::m_instance = NULL;

ModelerDrawState::ModelerDrawState() : m_drawMode(NORMAL), m_quality(MEDIUM),
showMarkers(false)
{
    float grey[]  = {.5f, .5f, .5f, 1};
    float white[] = {1,1,1,1};
    float black[] = {0,0,0,1};
    
    memcpy(m_ambientColor, black, 4 * sizeof(float));
    memcpy(m_diffuseColor, grey, 4 * sizeof(float));
    memcpy(m_specularColor, white, 4 * sizeof(float));
    
    m_shininess = 5.5;
    
    m_rayFile = NULL;
}

// CLASS ModelerDrawState METHODS
ModelerDrawState* ModelerDrawState::Instance()
{
    // Return the singleton if it exists, otherwise, create it
    return (m_instance) ? (m_instance) : m_instance = new ModelerDrawState();
}

// ****************************************************************************
// Modeler functions for your use
// ****************************************************************************
// Set the current material properties

void setAmbientColor(float r, float g, float b)
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    mds->m_ambientColor[0] = (GLfloat)r;
    mds->m_ambientColor[1] = (GLfloat)g;
    mds->m_ambientColor[2] = (GLfloat)b;
    mds->m_ambientColor[3] = (GLfloat)1.0;
    
    if (mds->m_drawMode == NORMAL)
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mds->m_ambientColor);
}

void setEmissiveColor(float r, float g, float b)
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    mds->m_emissiveColor[0] = (GLfloat)r;
    mds->m_emissiveColor[1] = (GLfloat)g;
    mds->m_emissiveColor[2] = (GLfloat)b;
    mds->m_emissiveColor[3] = (GLfloat)1.0;
    
    if (mds->m_drawMode == NORMAL)
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mds->m_emissiveColor);
}

void setDiffuseColor(float r, float g, float b)
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    mds->m_diffuseColor[0] = (GLfloat)r;
    mds->m_diffuseColor[1] = (GLfloat)g;
    mds->m_diffuseColor[2] = (GLfloat)b;
    mds->m_diffuseColor[3] = (GLfloat)1.0;
    
    if (mds->m_drawMode == NORMAL)
        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, mds->m_diffuseColor);
    else
        glColor3f(r,g,b);
}

void setSpecularColor(float r, float g, float b)
{	
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    mds->m_specularColor[0] = (GLfloat)r;
    mds->m_specularColor[1] = (GLfloat)g;
    mds->m_specularColor[2] = (GLfloat)b;
    mds->m_specularColor[3] = (GLfloat)1.0;
    
    if (mds->m_drawMode == NORMAL)
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, mds->m_specularColor);
}

void setShininess(float s)
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    mds->m_shininess = (GLfloat)s;
    
    if (mds->m_drawMode == NORMAL)
        glMaterialf( GL_FRONT, GL_SHININESS, mds->m_shininess);
}

void setDrawMode(DrawModeSetting_t drawMode)
{
    ModelerDrawState::Instance()->m_drawMode = drawMode;
}

void setQuality(QualitySetting_t quality)
{
    ModelerDrawState::Instance()->m_quality = quality;
}

bool openRayFile(const char rayFileName[])
{
    ModelerDrawState *mds = ModelerDrawState::Instance();

	fprintf(stderr, "Warning: Ray file may render differently.\n");
    
    if (!rayFileName)
        return false;
    
    if (mds->m_rayFile) 
        closeRayFile();
    
    //mds->m_rayFile = fopen(rayFileName, "w");
	mds->m_rayFile = NULL;
	fopen_s(&(mds->m_rayFile), rayFileName, "w");
    
    if (mds->m_rayFile != NULL) 
    {
		fprintf( mds->m_rayFile, "SBT-raytracer 1.0\n\n" );

		
        return true;
    }
    else
        return false;
}

static void _setupOpenGl()
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
	switch (mds->m_drawMode)
	{
	case NORMAL:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glShadeModel(GL_SMOOTH);
		break;
	case FLATSHADE:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glShadeModel(GL_FLAT);
		break;
	case WIREFRAME:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glShadeModel(GL_FLAT);
	default:
		break;
	}

}

void closeRayFile()
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    
    if (mds->m_rayFile) 
        fclose(mds->m_rayFile);
    
    mds->m_rayFile = NULL;
}

void drawAxes()
{
	// Draw the origin
	glPushMatrix();
	glScalef(5,5,5);

	glShadeModel(GL_FLAT);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
		// x axis
		glColor3f(1.0, 0.0, 0.0);
		glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 1.0, 0.0, 0.0 );

		// y axis
		glColor3f(0.0, 1.0, 0.0);
		glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 0.0, 1.0, 0.0 );

		// z axis
		glColor3f(0.0, 0.0, 1.0);
		glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 0.0, 0.0, 1.0 );
	glEnd();

	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos3f(1.2f, 0.0f, 0.0f);
	printString("X");

	glColor3f(0.0, 1, 0.0);
	glRasterPos3f(0.0f, 1.2f, 0.0f);
	printString("Y");

	glColor3f(0.0f, 0.0f, 1.0f);
	glRasterPos3f(0.0f, 0.0f, 1.2f);
	printString("Z");

	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void drawSphere(double r)
{
    ModelerDrawState *mds = ModelerDrawState::Instance();

	_setupOpenGl();
    
    if (mds->m_rayFile)
    {
        _dump_current_modelview();
        fprintf(mds->m_rayFile, "scale(%f,%f,%f,sphere {\n", r, r, r );
        _dump_current_material();
        fprintf(mds->m_rayFile, "}))\n\n" );
    }
    else
    {
        int divisions; 
        switch(mds->m_quality)
        {
        case HIGH: 
            divisions = 128; break;
        case MEDIUM: 
            divisions = 32; break;
        case LOW:
            divisions = 16; break;
        case POOR:
            divisions = 8; break;
        }

		GLUquadricObj* gluq = gluNewQuadric();
        gluQuadricDrawStyle( gluq, GLU_FILL );
		gluQuadricNormals( gluq, GLU_SMOOTH );
		glShadeModel(GL_SMOOTH);
        gluQuadricTexture( gluq, GL_TRUE );
		
        gluSphere(gluq, r, divisions, divisions);
		
		glShadeModel(GL_SMOOTH);
        gluDeleteQuadric( gluq );
    }
}

int load_2dcurve_txt(const char* filename,std::vector<Point2d>* pts) {
	std::ifstream infile( filename );
	if ( infile.is_open() ) {

		pts->clear();

		float x,y;
		while ( (infile >> x) && (infile >> y) ) {
			Point2d p;
			p.x = x;
			p.y = y;
			pts->push_back( p );
		}
		infile.close();
	} else {
		return -1;
	}

	return 0;
}


std::vector<Point2d> revolution_pts;

int reload_curve_file(const char* filename) {
	int load_ret = load_2dcurve_txt( filename, &revolution_pts );
	if ( load_ret < 0 ) {
		printf( "Cannot open curve file: %s.\n", filename );
	}

	return load_ret;
}

void init_load_curve_file() {
	int load_ret = load_2dcurve_txt( "curves/sample1_curve.apts", &revolution_pts );
	if ( load_ret < 0 ) {
		printf( "Cannot open curve file.\n" );
		return;
	}
}

// rayfile: the target file to write
// num_vertices: the number of input vertices in the mesh
// num_triangles: the number of triangles in the mesh
// vertices: vertices coordinates in an array of (3 x num_vertices) floating number, the formation is p[0].x, p[0].y, p[0].z, p[1].x, p[1].y, p[1].z, ...
// normals: vertex normals in an array of (3 x num_vertices) floating numbers, the formation is n[0].x, n[0].y, n[0].z, n[1].x, n[1].y, n[1].z, ...
// trindices: triangle indices in an array of (3 x num_triangles) unsigned integers, the formation is 
// triangles[0].p[0], triangles[0].p[1], triangles[0].p[2], triangles[1].p[0], triangles[1].p[1], triangles[1].p[2], ...
// texture_uv: texture coordinates stored in a similar way
int write_revolution_rayfile(FILE* rayfile, int num_vertices, int num_triangles, GLfloat* vertices, GLfloat* normals, GLuint* trindices, GLfloat* texture_uv) {
	
	if ( rayfile == NULL ) {
		return -1;
	}
	if ( (num_vertices<=0) || (num_triangles<=0) ) {
		return -2;
	}
	if ( (vertices==NULL)||(normals==NULL)||(trindices==NULL) ) {
		return -3;
	}
	
	// write into rayfile
	//_dump_current_modelview();
	fprintf(rayfile, "polymesh {\nname=\"revolution\";\n" );
	_dump_current_material();

	fprintf(rayfile, "points=( " );
	GLfloat* pvertices = vertices;
	
	for ( int j=0; j<num_vertices; ++j ) {
		fprintf(rayfile, "( %f,%f,%f ),", *pvertices, *(pvertices+1), *(pvertices+2) );
		pvertices += 3;
	}
	fseek(rayfile, -1, SEEK_END);
	fprintf(rayfile, ");\n" );

	fprintf(rayfile, "faces=( " );
	GLuint* ptrindices = trindices;
	for ( int j=0; j<num_triangles; ++j ) {
		fprintf(rayfile, "( %d,%d,%d ),", *ptrindices, *(ptrindices+1), *(ptrindices+2) );
		ptrindices += 3;
	}
	fseek(rayfile, -1, SEEK_END);
	fprintf(rayfile, ");\n" );

	fprintf(rayfile, "normals=( " );
	GLfloat* pnormals = normals;
	for ( int j=0; j<num_vertices; ++j ) {
		fprintf(rayfile, "( %f,%f,%f ),", *pnormals, *(pnormals+1), *(pnormals+2) );
		pnormals += 3;
	}
	fseek(rayfile, -1, SEEK_END);
	fprintf(rayfile, ");\n" );

	fprintf(rayfile, "texture_uv=( " );
	GLfloat* puv = texture_uv;
	for ( int j=0; j<num_vertices; ++j ) {
		fprintf(rayfile, "( %f,%f),", *puv, *(puv+1));
		puv += 2;
	}
	fseek(rayfile, -1, SEEK_END);
	fprintf(rayfile, ");\n" );

	fprintf(rayfile, "}\n\n" );

	return 0;
}

void drawRevolution(double scale)
{
	if ( revolution_pts.empty() ) {
		return;
	}
    ModelerDrawState *mds = ModelerDrawState::Instance();

	_setupOpenGl();
	if (mds->m_rayFile)
	{
		// write into rayfile
	} else    
    {
        int divisions; 
        switch(mds->m_quality)
        {
        case HIGH: 
            divisions = 128; break;
        case MEDIUM: 
            divisions = 32; break;
        case LOW:
            divisions = 16; break;
        case POOR:
            divisions = 8; break;
        }

		const float PI = 3.141592653589793238462643383279502f;

		float origin_y = 1.13132490911795f;
		float origin_z = -3.0f;

		int num_pts = revolution_pts.size();
		for ( int i=0; i<6; ++ i ) {
			// This implementation uses glBegin(GL_TRIANGLES)/glend() and has trivial normals and texture coordinates.
			// It is OK to start from the code below for developing and testing,
			// but please use glDrawElements() with GL_TRIANGES to draw the surface in the final code.
			// It is a naive version of surface of revolution created by translating a curve along a straight line.
			// You need to rotate the curve to create a more interesting shape. Also, please create your own curve with the curve editor tool. Good luck!						
			for ( int j=1; j<num_pts; ++ j ) {				

				float p1_x = (float)(revolution_pts[j-1].x * scale);
				float p1_y = (float)(revolution_pts[j-1].y * scale + origin_y);
				float p1_z = i+origin_z;

				float p2_x = (float)(revolution_pts[j].x * scale);
				float p2_y = (float)(revolution_pts[j].y * scale + origin_y);
				float p2_z = p1_z;

				float p3_x = p1_x;
				float p3_y = p1_y;
				float p3_z = p1_z+1;

				float p4_x = p2_x;
				float p4_y = p2_y;
				float p4_z = p3_z;

				// You must compute the normal directions add texture coordinates, see lecture notes
				float n1_x = 1;
				float n1_y = 0;
				float n1_z = 0;
				float texture_u = 1.0f;
				float texture_v = 1.0f;

				glBegin(GL_TRIANGLES);
					glNormal3f(n1_x,n1_y,n1_z);
					glTexCoord2f( texture_u, texture_v);
					glVertex3f(p1_x, p1_y, p1_z);
					
					glNormal3f(n1_x,n1_y,n1_z);
					glTexCoord2f( texture_u, texture_v);
					glVertex3f(p2_x, p2_y, p2_z);

					glNormal3f(n1_x,n1_y,n1_z);
					glTexCoord2f( texture_u, texture_v);
					glVertex3f(p3_x, p3_y, p3_z);
				glEnd();

				glBegin(GL_TRIANGLES);
					glNormal3f(n1_x,n1_y,n1_z);
					glTexCoord2f( texture_u, texture_v);
					glVertex3f(p3_x, p3_y, p3_z);					
					
					glNormal3f(n1_x,n1_y,n1_z);
					glTexCoord2f( texture_u, texture_v);
					glVertex3f(p2_x, p2_y, p2_z);

					glNormal3f(n1_x,n1_y,n1_z);
					glTexCoord2f( texture_u, texture_v);
					glVertex3f(p4_x, p4_y, p4_z);
				glEnd();
			}			
		}
		
    }
}


void drawBox( double x, double y, double z )
{
    ModelerDrawState *mds = ModelerDrawState::Instance();

	_setupOpenGl();
    
    if (mds->m_rayFile)
    {
        _dump_current_modelview();
        fprintf(mds->m_rayFile,  
            "scale(%f,%f,%f,translate(0.5,0.5,0.5,box {\n", x, y, z );
        _dump_current_material();
        fprintf(mds->m_rayFile,  "})))\n\n" );
    }
    else
    {
        /* remember which matrix mode OpenGL was in. */
        int savemode;
        glGetIntegerv( GL_MATRIX_MODE, &savemode );
        
        /* switch to the model matrix and scale by x,y,z. */
        glMatrixMode( GL_MODELVIEW );

        glPushMatrix();
        glScaled( x, y, z );
        
        glBegin( GL_QUADS );
        
        glNormal3d( 0.0, 0.0, -1.0 );
        glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 0.0, 1.0, 0.0 );
        glVertex3d( 1.0, 1.0, 0.0 );
		glVertex3d( 1.0, 0.0, 0.0 );
        
        glNormal3d( 0.0, -1.0, 0.0 );
        glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 1.0, 0.0, 0.0 );
        glVertex3d( 1.0, 0.0, 1.0 );
		glVertex3d( 0.0, 0.0, 1.0 );
        
        glNormal3d( -1.0, 0.0, 0.0 );
        glVertex3d( 0.0, 0.0, 0.0 );
		glVertex3d( 0.0, 0.0, 1.0 );
        glVertex3d( 0.0, 1.0, 1.0 );
		glVertex3d( 0.0, 1.0, 0.0 );
        
        glNormal3d( 0.0, 0.0, 1.0 );
        glVertex3d( 0.0, 0.0, 1.0 );
		glVertex3d( 1.0, 0.0, 1.0 );
        glVertex3d( 1.0, 1.0, 1.0 );
		glVertex3d( 0.0, 1.0, 1.0 );
        
        glNormal3d( 0.0, 1.0, 0.0 );
        glVertex3d( 0.0, 1.0, 0.0 );
		glVertex3d( 0.0, 1.0, 1.0 );
        glVertex3d( 1.0, 1.0, 1.0 );
		glVertex3d( 1.0, 1.0, 0.0 );
        
        glNormal3d( 1.0, 0.0, 0.0 );
        glVertex3d( 1.0, 0.0, 0.0 );
		glVertex3d( 1.0, 1.0, 0.0 );
        glVertex3d( 1.0, 1.0, 1.0 );
		glVertex3d( 1.0, 0.0, 1.0 );
        
        glEnd();
        
        /* restore the model matrix stack, and switch back to the matrix
        mode we were in. */
        glPopMatrix();
        glMatrixMode( savemode );
    }
}

void drawCylinder( double h, double r1, double r2 )
{
    ModelerDrawState *mds = ModelerDrawState::Instance();
    int divisions;

	_setupOpenGl();
    
    switch(mds->m_quality)
    {
    case HIGH: 
        divisions = 32; break;
    case MEDIUM: 
        divisions = 20; break;
    case LOW:
        divisions = 12; break;
    case POOR:
        divisions = 8; break;
    }
    
    if (mds->m_rayFile)
    {
        _dump_current_modelview();
        fprintf(mds->m_rayFile, 
            "cone { height=%f; bottom_radius=%f; top_radius=%f; capped = true; \n", h, r1, r2 );
        _dump_current_material();
        fprintf(mds->m_rayFile, "})\n\n" );
    }
    else
    {
		//////// BEGIN YOUR CYLINDER CODE /////////
		// ... if you are replacing the cylinder.
        GLUquadricObj* gluq;
        
        /* GLU will again do the work.  draw the sides of the cylinder. */
        gluq = gluNewQuadric();
        gluQuadricDrawStyle( gluq, GLU_FILL );
        gluQuadricTexture( gluq, GL_TRUE );
        gluCylinder( gluq, r1, r2, h, divisions, divisions);
        gluDeleteQuadric( gluq );
        
        if ( r1 > 0.0 )
        {
        /* if the r1 end does not come to a point, draw a flat disk to
            cover it up. */
            
            gluq = gluNewQuadric();
            gluQuadricDrawStyle( gluq, GLU_FILL );
            gluQuadricTexture( gluq, GL_TRUE );
            gluQuadricOrientation( gluq, GLU_INSIDE );
            gluDisk( gluq, 0.0, r1, divisions, divisions);
            gluDeleteQuadric( gluq );
        }
        
        if ( r2 > 0.0 )
        {
        /* if the r2 end does not come to a point, draw a flat disk to
            cover it up. */
            
            /* save the current matrix mode. */	
            int savemode;
            glGetIntegerv( GL_MATRIX_MODE, &savemode );
            
            /* translate the origin to the other end of the cylinder. */
            glMatrixMode( GL_MODELVIEW );
            glPushMatrix();
            glTranslated( 0.0, 0.0, h );
            
            /* draw a disk centered at the new origin. */
            gluq = gluNewQuadric();
            gluQuadricDrawStyle( gluq, GLU_FILL );
            gluQuadricTexture( gluq, GL_TRUE );
            gluQuadricOrientation( gluq, GLU_OUTSIDE );
            gluDisk( gluq, 0.0, r2, divisions, divisions);
            gluDeleteQuadric( gluq );
            
            /* restore the matrix stack and mode. */
            glPopMatrix();
            glMatrixMode( savemode );
        }
		//////// END YOUR CYLINDER CODE /////////
    }
}

// In opengl, triangles have a "front" and "back".  This is for
// efficiency purposes.  You will want to make sure that you
// specify the vertices (x1,y1,z1), (x2,y2,z2), (x3,y3,z3) in
// *counterclockwise* order.
void drawTriangle( double x1, double y1, double z1,
                   double x2, double y2, double z2,
                   double x3, double y3, double z3 )
{
    ModelerDrawState *mds = ModelerDrawState::Instance();

	_setupOpenGl();

    if (mds->m_rayFile)
    {
        _dump_current_modelview();
        fprintf(mds->m_rayFile, 
            "polymesh { points=((%f,%f,%f),(%f,%f,%f),(%f,%f,%f)); faces=((0,1,2));\n", x1, y1, z1, x2, y2, z2, x3, y3, z3 );
        _dump_current_material();
        fprintf(mds->m_rayFile, "})\n\n" );
    }
    else
    {
        double a, b, c, d, e, f;
        
        /* the normal to the triangle is the cross product of two of its edges. */
        a = x2-x1;
        b = y2-y1;
        c = z2-z1;
        
        d = x3-x1;
        e = y3-y1;
        f = z3-z1;
        
        glBegin( GL_TRIANGLES );
        glNormal3d( b*f - c*e, c*d - a*f, a*e - b*d );
        glVertex3d( x1, y1, z1 );
        glVertex3d( x2, y2, z2 );
        glVertex3d( x3, y3, z3 );
        glEnd();
    }
}












