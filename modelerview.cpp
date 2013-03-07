#include "modelerview.h"
#include "modelerdraw.h"
#include "camera.h"

#include <time.h>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Gl_Window.h>
//#include <FL/gl.h>
//#include <GL/glu.h>
#include <cstdio>

static const int	kMouseRotationButton			= FL_LEFT_MOUSE;
static const int	kMouseTranslationButton			= FL_MIDDLE_MOUSE;
static const int	kMouseZoomButton				= FL_RIGHT_MOUSE;

static const int	kMouseTranslationKey			= FL_SHIFT;
static const int	kMouseZoomKey					= FL_CTRL;

//static const int    LINE_LENGTH                     = 70;

ModelerView::ModelerView(int x, int y, int w, int h, char *label)
	: Fl_Gl_Window(x,y,w,h,label), m_camera(new Camera(w, h)), model(NULL),
	reloadResources(true), failed(false)
{}

ModelerView::~ModelerView()
{
	if (m_camera)
	{
		delete m_camera;
	}
}

void ModelerView::retry() {
	reloadResources = true;
	redraw();
}

void ModelerView::setModel(Model* model) {
	this->model = model;
	redraw(); // TODO: is this desirable?
}

int ModelerView::handle(int event)
{
    unsigned eventCoordX = Fl::event_x();
	unsigned eventCoordY = Fl::event_y();
	unsigned eventButton = Fl::event_button();
	unsigned eventState  = Fl::event_state();

	switch(event)	 
	{
	case FL_PUSH:
		{
			switch(eventButton)
			{
			case kMouseRotationButton:
				if (Fl::event_state() & kMouseTranslationKey) {
					m_camera->clickMouse(kActionTranslate, eventCoordX, eventCoordY );
				} else if (Fl::event_state() & kMouseZoomKey) {
					m_camera->clickMouse(kActionZoom, eventCoordX, eventCoordY );
				} else {
					m_camera->clickMouse(kActionRotate, eventCoordX, eventCoordY );
				}
				break;
			case kMouseTranslationButton:
				m_camera->clickMouse(kActionTranslate, eventCoordX, eventCoordY );
				break;
			case kMouseZoomButton:
				m_camera->clickMouse(kActionZoom, eventCoordX, eventCoordY );
				break;
			}
           // printf("push %d %d\n", eventCoordX, eventCoordY);
		}
		break;
	case FL_DRAG:
		{
			m_camera->dragMouse(eventCoordX, eventCoordY);
            //printf("drag %d %d\n", eventCoordX, eventCoordY);
		}
		break;
	case FL_RELEASE:
		{
			switch(eventButton)
			{
			case kMouseRotationButton:
			case kMouseTranslationButton:
			case kMouseZoomButton:
				m_camera->releaseMouse(eventCoordX, eventCoordY );
				break;
			}
          //  printf("release %d %d\n", eventCoordX, eventCoordY);
		}
		break;
	default:
		return 0;
	}
	
	redraw();

	return 1;
}

void ModelerView::draw() {
	// The critical section between failed = true and failed = false must run
	// successfully if the rest of the model is to be drawn.
	if (failed) return;
	failed = true;
    if (!valid())
    {
        glShadeModel( GL_SMOOTH );
        glEnable( GL_DEPTH_TEST );
		glEnable( GL_NORMALIZE );

		// Load GLEW to support shaders.
		// TODO: display errors in message boxes!
		GLenum err = glewInit();
		if (GLEW_OK != err)
		{
			fl_alert("GLEW failed to initialize:\n%s\n\n"
				"Please make sure you have OpenGL compatible\nhardware and are not under remote desktop.", glewGetErrorString(err));
			return;
		}
		else if (!GLEW_ARB_vertex_shader)
		{
			fl_alert("Your hardware does not support vertex shaders.\n"
				"Please make sure you have OpenGL compatible\nhardware and are not under remote desktop.");
			return;
		}
		else if (!GLEW_ARB_fragment_shader)
		{
			fl_alert("Your hardware does not support fragment (pixel) shaders."
				"Please make sure you have OpenGL compatible\nhardware and are not under remote desktop.");
			return;
		}
	}
	failed = false;

	// Reload textures and shaders if desired.
	if (reloadResources) {
		reloadResources = false;
		try {
			model->load();

			if (shaderSourceCode) {
				// Make random key
				static const char chars[] =
					"abcdefghijklmnopqrstuvwxyz"
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					"0123456789";
				const int CHARS_COUNT = 62;
				const int KEY_LENGTH = 40;
				char key[KEY_LENGTH + 1];
				key[KEY_LENGTH] = 0;
				srand((unsigned int)time(NULL));
				for (int i = 0; i < KEY_LENGTH; i++) {
					key[i] = chars[rand() % CHARS_COUNT];
				}

				// Write key and shader count
				ofstream shaders("shaders.cpp");
				shaders << "#include \"vault.h\"" << endl << "const char* SHADER_KEY = \""
					<< key << "\";"
					<< endl << "int SHADER_COUNT = " << shaderSourceCode->size()
					<< ";";

				if (shaderSourceCode->size()) {
					// Write filenames
					shaders << endl << "const char* SHADER_FILENAMES[] = {";
					bool first = true;
					for (map<string, string>::iterator iter = shaderSourceCode->begin();
						iter != shaderSourceCode->end(); iter++) {
						if (!first) { shaders << ","; }
						shaders << endl << "\"" << iter->first << "\"";
						first = false;
					}
					shaders << endl << "};";

					// Write data
					shaders << endl << "const char* SHADER_DATA[] = {";
					first = true;
					for (map<string, string>::iterator iter = shaderSourceCode->begin();
						iter != shaderSourceCode->end(); iter++) {
						const char* buffer = encrypt(iter->second.c_str(), key);
						if (!first) { shaders << ","; }
						int len = strlen(buffer), pos, LINE_LENGTH = 70;
						for (pos = 0; pos < len; pos += LINE_LENGTH) {
							shaders << endl << "\"";
							shaders.write(buffer + pos,
								((len - pos) < LINE_LENGTH) ? len - pos : LINE_LENGTH);
							shaders << "\"";
						}
						first = false;
						delete buffer;
					}
					shaders << endl << "};";
				} else {
					shaders << endl << "const char* SHADER_FILENAMES[1];"
						<< endl << "const char* SHADER_DATA[1];";
				}

				shaders.close();
				delete shaderSourceCode;
				shaderSourceCode = NULL;
			}
		} catch (...) {
			// Assume that an alert was shown if there was an error anywhere.
			return;
		}
	}	

	// Resize the viewport if needed
	m_camera->resizeViewport(w(),h());

	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up the projection matrix
	m_camera->applyProjectionTransform();
	
	// Set up the modelview matrix
    m_camera->applyViewingTransform();

	// Draw the axes
	if (ModelerDrawState::Instance()->showMarkers) { drawAxes(); }

	// Draw the particle system
	ModelerUserInterface* ui = ModelerUserInterface::getInstance();
	ParticleSystem* ps = ui->getParticleSystem();
	if (ps) {
		ps->computeForcesAndUpdateParticles(ui->getTime());
		ps->drawParticles(ui->getTime());
	}

	// Draw the model
	model->draw();
}