#include <cctype>
#include "modelerui.h"
#include "modelerdraw.h"

using namespace std;


// A special FLTK tiling window that keeps the bottom window the same size.
class Special_Tile : public Fl_Tile {
public:
	Special_Tile(int x, int y, int w, int h, const char *L=0):
		Fl_Tile(x,y,w,h,L){};

    void resize(int x, int y, int w, int h) {
		Fl_Group::resize(x, y, w, h);
	}
};

// List of curve types for the dropdown menu.
// TODO: handle this in a more automatic fashion?
Fl_Menu_Item ModelerUserInterface::curveTypeMenu[] = {
 {"Linear", 0,  0, 0, 0, 0, 0, 12, 0},
 {"B-Spline", 0,  0, 0, 0, 0, 0, 12, 0},
 {"Bezier", 0,  0, 0, 0, 0, 0, 12, 0},
 {"Catmull-Rom", 0,  0, 0, 0, 0, 0, 12, 0},
 {"C2-Interpolating", 0,  0, 0, 0, 0, 0, 12, 0},
 {0}
};


ModelerUserInterface* ModelerUserInterface::instance;

void ModelerUserInterface::cb_m_controlsWindow_i(Fl_Double_Window* o, void*) {
  ModelerUserInterface* pUI = (ModelerUserInterface*)(o->user_data());
	if(Fl::e_number == FL_CLOSE) {
		pUI->m_controlsWindow->hide();
	};
}
void ModelerUserInterface::cb_m_controlsWindow(Fl_Double_Window* o, void* v) {
  ((ModelerUserInterface*)(o->user_data()))->cb_m_controlsWindow_i(o,v);
}

void ModelerUserInterface::cb_Save_i(Fl_Menu_*, void*) {
  const char *filename = fileDialog(Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
				"RAY File (*.ray)\t*.ray",
				"Save Raytracer File");
	
	if(!filename)
		return;
	
	if(openRayFile(filename) == false) {
		fl_alert("Error opening file.");
	}
	else {
		m_modelerView->draw();
		closeRayFile();
	};
}

void ModelerUserInterface::cb_Save(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Save_i(o,v);
}

void ModelerUserInterface::cb_SaveMovie(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->saveMovie();
}

void ModelerUserInterface::cb_LoadScript(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->loadScript();
}

void ModelerUserInterface::cb_SaveScript(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->saveScript();
}

int ModelerUserInterface::loadFile(std::string& buffer,
								   const bool save,
								   const char* title,
								   const char* filter,
								   const char* extensions) {
	while (true) {
		// Ask user
		const char* fileName = fileDialog(
			save ? Fl_Native_File_Chooser::BROWSE_SAVE_FILE :
				Fl_Native_File_Chooser::BROWSE_FILE,
			filter, title);

		if (fileName == NULL) {
			return 0;
		}
		buffer = std::string(fileName);

		// Get desired file type
		int fileType = m_nativeChooser->filter_value();
		int retVal = fileType + 1;

		if (save) {
			// Get the extension for the file type
			const char* ext = extensions - 1;
			while (ext && fileType) {
				fileType--;
				ext = strchr(ext + 1, '\t');
			}

			// Append extension if the file name is missing one.
			if (ext) {
				ext++;
				int end = strlen(ext);
				const char* extEnd = strchr(ext, '\t');
				if (extEnd != NULL) {
					end -= strlen(extEnd);
				}
				std::string realExt(ext, end);

				unsigned int pos = buffer.find_last_of('.');
				if (pos == buffer.npos || buffer.find_last_of("\\/") > pos) {
					buffer.append(".");
					buffer.append(realExt);
				} else {
					std::string bufferExt = buffer.substr(pos + 1);
					// Make sure the extension matches the file type.
					if (bufferExt.compare(realExt) != 0) {
						switch (fl_choice("You selected the file type %s, but your "
							"file name's extension is %s.\n\nDo you want to fix "
							"the extension, or save with a different name?",
							"Cancel", "Fix Extension", "Different Name",
							realExt.c_str(), bufferExt.c_str())) {
						case 0:
							return 0;
						case 1:
							buffer = buffer.substr(0, pos + 1);
							buffer += realExt;
							break;
						case 2:
							continue;
						}
					}
				}
			}
		}

		return retVal;
	}
}

void ModelerUserInterface::loadScript() {
	// Ask user for filename
	std::string fileName;
	if (!loadFile(fileName, false, "Load Animation Script",
		"Animation Scripts (*.ani)\t*.ani", "ani")) { return; }

	// Attempt to load the model script
	if (!graph->loadModelScript(fileName.c_str())) {
		fl_alert("Error loading the model animation script.");
	}
}

void ModelerUserInterface::saveScript() {
	// Ask user for filename
	std::string fileName;
	if (!loadFile(fileName, true, "Save Animation Script",
		"Animation Scripts (*.ani)\t*.ani", "ani")) { return; }

	// Attempt to save the model script
	if (!graph->saveModelScript(fileName.c_str())) {
		fl_alert("Error saving the model animation script.");
	}
}

void ModelerUserInterface::finishSavingMovie() {
	if (!rendering) { return; }

	// Restore the original interface
	rendering = false;

	m_controlsWindow->resizable(m_viewPane);

	m_controlsMenuBar->show();

	m_modelerView->resize(250, 25, m_controlsWindow->w() - 250,
		curvePane->y() - 25);
	m_modelerView->activate();
	
	leftPane->show();
	curvePane->show();

	m_viewPane->init_sizes();
	m_controlsWindow->init_sizes();

	m_controlsWindow->size(oldWidth, oldHeight);
}

void ModelerUserInterface::cancelRender(Fl_Button* button, ModelerUserInterface* ui) {
	ui->stop();
}

void ModelerUserInterface::setResolution() {
	int width, height;

	// Prompt for width.
	const char* szWidth = NULL;
	do {
		szWidth = fl_input("New Animation Width (in pixels) (300 ~ 2500)", "");

		if (szWidth) {
			width = atoi(szWidth);
		} else {
			return;
		}
	} while (szWidth && (width < 300 || width > 2500));

	// Prompt for height.
	const char* szHeight = NULL;
	do {
		szHeight = fl_input("New Animation Height (in pixels) (200 ~ 2500)", "");

		if (szHeight) {
			height = atoi(szHeight);
		} else {
			return;
		}
	} while (szHeight && (height < 200 || height > 2500));

	// Set width and height only if both were provided.
	movieWidth = width; movieHeight = height;
}

void ModelerUserInterface::cb_setResolution(Fl_Menu_* o, void* v) {
	((ModelerUserInterface*)(o->parent()->user_data()))->setResolution();
}

void ModelerUserInterface::saveMovie() {
	// Don't save the movie while one is being saved.
	if (rendering) { return; }

	// Get file name and type.
	std::string fileName;
	int fileType = loadFile(fileName, true, "Save Movie As",
		"PNG Image File (*.png)\t*.png\nJPEG Image File (*.jpg)\t*.jpg",
		"png\tjpg");
	if (!fileType) { return; }
	renderFileType = fileType - 1;
	int extPos = fileName.find_last_of('.');
	renderFileName = fileName.substr(0, extPos);
	renderFileExt = fileName.substr(extPos);

	// Set JPEG rendering quality.
	renderJPEGQuality = 95;
	if (fileType == 2) {
		Dialog2 x(0,0,0,0,"ok");
		renderJPEGQuality = x.getValue();
	}

	// Stop and rewind.
	rewind();
	
	// Hide all controls and make the viewing window fill the screen.
	oldWidth = m_controlsWindow->w();
	oldHeight = m_controlsWindow->h();
	m_controlsWindow->size(movieWidth, movieHeight + 25);
	m_controlsMenuBar->hide();
	leftPane->hide();
	curvePane->hide();
	m_modelerView->resize(0, 25, m_controlsWindow->w(), m_controlsWindow->h() - 25);
	m_modelerView->deactivate(); // prevent user input

	// Show some rendering controls
	renderGroup = new Fl_Group(5, 5, m_controlsWindow->w(), 15);
		// Status text
		Fl_Box* box = new Fl_Box(5, 5, 155, 15, "Rendering animation...");
		box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		// Render progress bar
		renderProgress = new Fl_Progress(155, 5, m_controlsWindow->w() - 215, 15);
		renderProgress->minimum(0);
		renderProgress->value(0);
		renderProgress->maximum(graph->endTime());
		renderProgress->selection_color(FL_BLUE);

		// Cancel button
		Fl_Button* buttonCancelRender = new Fl_Button(m_controlsWindow->w() - 55, 5, 50, 15, "Cancel");
		buttonCancelRender->callback((Fl_Callback*)cancelRender, this);

		// End this group
		renderGroup->end();

		// Add group to the window
		m_controlsWindow->add(renderGroup);

	// Initialize sizes and disable resizing
	m_controlsWindow->init_sizes();
	m_viewPane->init_sizes();
	m_controlsWindow->resizable(NULL);

	// Turn off light and camera markers
	ModelerDrawState::Instance()->showMarkers = false;

	// Enable particle system simulation
	setSimulate(true);

	// Begin rendering
	rendering = true;
	renderFrame = 0;
	play();
}

void ModelerUserInterface::saveFrame(const char* filename, int fileType,
									 int quality) {
	int x = m_modelerView->x();
	int y = m_modelerView->y();
	int w = m_modelerView->w();
	int h = m_modelerView->h();
	
	m_modelerView->make_current();
	m_modelerView->draw();
		
	unsigned char *imageBuffer = new unsigned char[3*w*h];
	
	// Have OpenGL read pixels from its back buffer
	glReadBuffer(GL_BACK);
	
	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glPixelStorei( GL_PACK_ROW_LENGTH, w );
	glReadPixels( 0, 0, w, h, 
		GL_RGB, GL_UNSIGNED_BYTE,
		imageBuffer );
	
	// Save the image
	saveImage(filename, imageBuffer, w, h, fileType == 0 ? ".png" : ".jpg", quality);
	
	delete [] imageBuffer;
}

/**
 * Save a screenshot.
 */
void ModelerUserInterface::cb_Save1_i(Fl_Menu_*, void*) {
	try {
		// Ask the user where to save the screenshot, and in what format.
		std::string fileName;
		int fileType = loadFile(fileName, true, "Save Screenshot",
			"PNG Image File (*.png)\t*.png\nJPEG Image File (*.jpg)\t*.jpg",
			"png\tjpg");

		// Exit if the user cancelled the dialog.
		if (!fileType) {
			return;
		}

		// Get quality setting for JPEG images.
		int quality = 95;
		if (fileType == 2) {
			Dialog2 x(0,0,0,0,"ok");
			quality = x.getValue();
		}

		// Take the screenshot and save it.
		saveFrame(fileName.c_str(), fileType - 1, quality);
	} catch (...) {
		// Assume that errors were reported when thrown...
	}
}
void ModelerUserInterface::cb_Save1(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Save1_i(o,v);
}

void ModelerUserInterface::cb_loadcurve_i(Fl_Menu_*, void*) {

  const char *filename = fileDialog(Fl_Native_File_Chooser::BROWSE_FILE,
				"Dense Point Sample File (*.apts)\t*.apts",
				"Open Dense Point Sample File");
	
	if(!filename)
		return;

	reload_curve_file( filename );
	
	m_modelerView->redraw();
}

void ModelerUserInterface::cb_Open_i(Fl_Menu_*, void*) {
/* TODO: load position files again
  const char *filename = fileDialog(Fl_Native_File_Chooser::BROWSE_FILE,
				"Position File (*.pos)\t*.pos",
				"Open Position File");
	
	if(!filename)
		return;
	
	std::ifstream ifs( filename );
	if( !ifs ) {
		std::cerr << "Error: couldn't read position file " << filename << std::endl;
		return;
	}
	
	float dolly, x, y, z;
	float quat[4];
	ifs >> dolly >> x >> y >> z >> quat[0] >> quat[1] >> quat[2] >> quat[3];
	
	m_modelerView->m_camera->setDolly( dolly );
	m_modelerView->m_camera->setLookAt( Vec3f(x, y, z) );
	m_modelerView->m_camera->setQuat( quat );
	
	unsigned int controlNum; 
	float value;
	while( ifs >> controlNum >> value ) {
		if( controlNum >= ModelerApplication::Instance()->GetNumControls() ) {
			break;
		}
		
		ModelerApplication::Instance()->SetControlValue(controlNum, value);
	}
	
	m_modelerView->redraw();*/
}

void ModelerUserInterface::cb_loadcurve(Fl_Menu_* o, void* v) {
	((ModelerUserInterface*)(o->parent()->user_data()))->cb_loadcurve_i(o,v);
}

void ModelerUserInterface::cb_Open(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Open_i(o,v);
}

void ModelerUserInterface::cb_Save2_i(Fl_Menu_*, void*) {
/* TODO: save position files again
  const char *filename = fileDialog(Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
				"Position File (*.pos)\t*.pos",
				"Save Position File");
	
	if(!filename)
		return;
	
	FILE* posFile = NULL;
	fopen_s(&posFile, filename, "w");
	if(!posFile) {
		fl_alert("Save failed!");
		return;
	}
	
	float dolly;
	const float* quat;
	Vec3f lookAt;
	dolly = m_modelerView->m_camera->getDolly();
	lookAt = m_modelerView->m_camera->getLookAt();
	quat = m_modelerView->m_camera->getQuat();
	
	fprintf(posFile, "%f %f %f %f %f %f %f %f\n", dolly, lookAt[0], lookAt[1], lookAt[2], quat[0], quat[1], quat[2], quat[3]);
	
	double value;
	for(unsigned int i = 0; i < ModelerApplication::Instance()->GetNumControls(); i++) {
		value = ModelerApplication::Instance()->GetControlValue(i);
		
		fprintf(posFile, "%d %f\n", i, value);
	}
	
	fclose(posFile);*/
}
void ModelerUserInterface::cb_Save2(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Save2_i(o,v);
}

void ModelerUserInterface::cb_Exit_i(Fl_Menu_*, void*) {
  m_controlsWindow->hide();
}
void ModelerUserInterface::cb_Exit(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Exit_i(o,v);
}

void ModelerUserInterface::cb_Normal_i(Fl_Menu_*, void*) {
  setDrawMode(NORMAL);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_Normal(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Normal_i(o,v);
}

void ModelerUserInterface::cb_Flat_i(Fl_Menu_*, void*) {
  setDrawMode(FLATSHADE);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_Flat(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Flat_i(o,v);
}

void ModelerUserInterface::cb_Wireframe_i(Fl_Menu_*, void*) {
  setDrawMode(WIREFRAME);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_Wireframe(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Wireframe_i(o,v);
}

void ModelerUserInterface::cb_High_i(Fl_Menu_*, void*) {
  setQuality(HIGH);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_High(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_High_i(o,v);
}

void ModelerUserInterface::cb_Medium_i(Fl_Menu_*, void*) {
  setQuality(MEDIUM);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_Medium(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Medium_i(o,v);
}

void ModelerUserInterface::cb_Low_i(Fl_Menu_*, void*) {
  setQuality(LOW);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_Low(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Low_i(o,v);
}

void ModelerUserInterface::cb_Poor_i(Fl_Menu_*, void*) {
  setQuality(POOR);
m_modelerView->redraw();
}
void ModelerUserInterface::cb_Poor(Fl_Menu_* o, void* v) {
  ((ModelerUserInterface*)(o->parent()->user_data()))->cb_Poor_i(o,v);
}

Fl_Menu_Item ModelerUserInterface::menu_m_controlsMenuBar[] = {
 {"File", 0,  0, 0, 64, FL_NORMAL_LABEL, 0, 14, 0},
 {"Save Raytracer File", 0,  (Fl_Callback*)ModelerUserInterface::cb_Save, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Save Image", 0,  (Fl_Callback*)ModelerUserInterface::cb_Save1, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
 // TODO: fix these and re-enable
// {"Open Position File", 0,  (Fl_Callback*)ModelerUserInterface::cb_Open, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
// {"Save Position File", 0,  (Fl_Callback*)ModelerUserInterface::cb_Save2, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
 {"Reload Textures and Shaders", 0, (Fl_Callback*)ModelerUserInterface::cb_Reload, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
#ifdef _DEBUG
 {"Generate shaders.cpp", 0, (Fl_Callback*)ModelerUserInterface::cb_GenShaders, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
#endif
 {"Save Movie Frames", 0, (Fl_Callback*)ModelerUserInterface::cb_SaveMovie, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Set Movie Dimensions", 0, (Fl_Callback*)ModelerUserInterface::cb_setResolution, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Set Frames Per Second", 0, (Fl_Callback*)ModelerUserInterface::cb_setFPS, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Set Movie Length", 0, (Fl_Callback*)ModelerUserInterface::cb_setMovieLength, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
 {"Open Animation Script", 0, (Fl_Callback*)ModelerUserInterface::cb_LoadScript, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Save Animation Script", 0, (Fl_Callback*)ModelerUserInterface::cb_SaveScript, 0, 128, FL_NORMAL_LABEL, 0, 14, 0},
 {"Exit", 0,  (Fl_Callback*)ModelerUserInterface::cb_Exit, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0},
 {"View", 0,  0, 0, 64, FL_NORMAL_LABEL, 0, 14, 0},
 {"Normal", 0,  (Fl_Callback*)ModelerUserInterface::cb_Normal, 0, 12, FL_NORMAL_LABEL, 0, 14, 0},
 {"Flat Shaded", 0,  (Fl_Callback*)ModelerUserInterface::cb_Flat, 0, 8, FL_NORMAL_LABEL, 0, 14, 0},
 {"Wireframe", 0,  (Fl_Callback*)ModelerUserInterface::cb_Wireframe, 0, 136, FL_NORMAL_LABEL, 0, 14, 0},
 {"High Quality", 0,  (Fl_Callback*)ModelerUserInterface::cb_High, 0, 8, FL_NORMAL_LABEL, 0, 14, 0},
 {"Medium Quality", 0,  (Fl_Callback*)ModelerUserInterface::cb_Medium, 0, 12, FL_NORMAL_LABEL, 0, 14, 0},
 {"Low Quality", 0,  (Fl_Callback*)ModelerUserInterface::cb_Low, 0, 8, FL_NORMAL_LABEL, 0, 14, 0},
 {"Poor Quality", 0,  (Fl_Callback*)ModelerUserInterface::cb_Poor, 0, 8 + 128, FL_NORMAL_LABEL, 0, 14, 0},
 {"Show Light/Camera Markers", 0, (Fl_Callback*)ModelerUserInterface::showMarkersCallback, 0, 2, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0}
};

/** Called when the Show Light/Camera Markers menu item is clicked. */
void ModelerUserInterface::showMarkersCallback(Fl_Menu_* m, void* p) {
	ModelerDrawState::Instance()->showMarkers =
		m->mvalue()->value() ? true : false;
	ModelerUserInterface::getInstance()->m_modelerView->redraw();
}

ModelerUserInterface::ModelerUserInterface() {
	// Make this instance the current one
	instance = this;

	// Initialize pointers to NULL
	m_nativeChooser = NULL;
	model = NULL;
	currentGroup = NULL;
	renderGroup = NULL;
	defaultCam = NULL;
	ps = NULL;
	movieWidth = 720;
	movieHeight = 480;

	// Set appearance to GTK+ for a nice look
	Fl::scheme("gtk+");

	// Set the animation speed to 24 frames/second
	framesPerSecond = 24;

	// We're not animating yet.
	animating = false;
	simulating = false;
	rendering = false;
	drawing = false;

	// Set the color scheme
	Fl::set_color(FL_BACKGROUND_COLOR, 240, 240, 240);
	Fl::set_color(FL_BACKGROUND2_COLOR, 255, 255, 255);
	Fl::set_color(FL_FOREGROUND_COLOR, 0, 0, 0);
	Fl::set_color(FL_INACTIVE_COLOR, 128, 128, 128);
	Fl::set_color(FL_SELECTION_COLOR, 51, 153, 255);

	// Create all of the UI elements
	// (autogenerated by FLUID, the FLTK UI Designer)
	Fl_Double_Window* w;
	//const char* title = "CSE 457 Animator";
	const char* title = "CSEP557 Animator";
	{ Fl_Double_Window* o = m_controlsWindow = new Fl_Double_Window(800, 625, title);
		w = o;
		o->callback((Fl_Callback*)cb_m_controlsWindow, (void*)(this));
		o->when(FL_WHEN_NEVER);
		{ Fl_Menu_Bar* o = m_controlsMenuBar = new Fl_Menu_Bar(0, 0, 800, 25);
		  o->menu(menu_m_controlsMenuBar);
		}

		// Contains the controls on the left
		{ leftPane = new Fl_Group(0, 25, 250, 600);
			int tabSpace = 0, controlSpace = 0;
			int controlTop = 25 + 600 - controlSpace;

			// Modeler and Curves tabs
			{ Fl_Tabs* t = new Fl_Tabs(0, 25, 250, 600 - controlSpace);
				// Make the tab area stretch.
				leftPane->resizable(t);

				// Curves tab
				{ Fl_Group* o = new Fl_Group(0, 50, 250, 575 - controlSpace, "Curves");
					o->box(FL_FLAT_BOX);
					o->color(FL_BACKGROUND_COLOR);
					{ Fl_Tree* o = curvesTree = new Fl_Tree(0, 50, 250, 575 - controlSpace);
					  o->when(FL_WHEN_CHANGED);
					  o->callback((Fl_Callback*)CurveTreeCallback);
					  o->marginleft(-5);
					  o->end();
					}
					o->end();
				}

				// Modeler tab
				{ Fl_Tile* o = m_controlSplitPane =
					new Fl_Tile(0, 50 + tabSpace, 250, 575 + tabSpace - controlSpace, "Modeler");
					// Make only the content area of the tabs resize.
					t->resizable(o);
					o->box(FL_FLAT_BOX);
					{ Fl_Tree* o = m_controlsTree = new Fl_Tree(0, 50 + tabSpace, 250, 100);
					  o->when(FL_WHEN_CHANGED);
					  o->callback((Fl_Callback*)TreeCallback);
					  o->marginleft(-5);
					  o->end();
					}
					{ Fl_Scroll* o = m_controlsScroll =
						new Fl_Scroll(0, 150 + tabSpace, 250, 475 - tabSpace - controlSpace);
					  o->type(Fl_Scroll::VERTICAL);
					  o->when(FL_WHEN_CHANGED);
					  { Fl_Pack* o = m_controlsPack =
						  new Fl_Pack(10, 150 + tabSpace, 215, 475 - tabSpace - controlSpace);
						Fl_Group::current()->resizable(o);
						o->spacing(2);
						o->end();
					  }
					  o->end();
					}
					o->end();
				} // end Modeler group/tab
				t->end();
			} // end tabs
			leftPane->end();
		} // left pane
		{ // TODO: remove this extra brace!
			{ Fl_Group* o = m_viewPane = new Special_Tile(250, 25, 550, 600);
				o->box(FL_NO_BOX);
				o->color(FL_BACKGROUND_COLOR);
				// show a smaller modeler view
				m_modelerView = new ModelerView(250, 25, 550, 350, "");

				// show a curve window with animation controls underneath
				{ curvePane = new Fl_Group(250, 375, 550, 250);
					

					// Row containing the curve options
					{ Fl_Group* o = new Fl_Group(255, 380, 540, 20, "Curve Editor");
						// Put particle system label inside
						o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
						o->labelfont(FL_HELVETICA_BOLD);

						// Curve type chooser
						chooserCurveType = new Fl_Choice(445, 380, 120, 20, "Curve Type: ");
						chooserCurveType->menu(curveTypeMenu);
						chooserCurveType->callback((Fl_Callback*)chooserCurveTypeCallback, this);
						chooserCurveType->deactivate();

						// Wrap checkbox
						checkboxWrap = new Fl_Check_Button(570, 380, 60, 20, "Wrap");
						checkboxWrap->callback((Fl_Callback*)checkboxWrapCallback, this);
						checkboxWrap->deactivate();

						// Zoom All button
						buttonZoomAll = new Fl_Button(640, 380, 80, 20, "Zoom All");
						buttonZoomAll->callback((Fl_Callback*)buttonZoomAllCallback, this);
						buttonZoomAll->deactivate();

						// No resizing
						o->resizable(NULL);

						o->end();
					}

					// The graph widget
					graph = new GraphWidget(255, 405, 540, 140);
					graph->callback((Fl_Callback*)graphCallback, this);
					curvePane->resizable(graph); // stretch the graph on resize

					// Camera buttons
					{ cameraPane = new Fl_Group(255, 550, 540, 20, "Camera");
						// Put camera label inside
						cameraPane->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
						cameraPane->labelfont(FL_HELVETICA_BOLD);

						// Prevent internal widgets from resizing when the group is resized.
						cameraPane->resizable(NULL);

						// Use Camera button
						checkboxUseCamera = new Fl_Check_Button(315, 550, 165, 20, "Look Through Camera");
						checkboxUseCamera->callback((Fl_Callback*)checkboxUseCameraCallback);

						// Plot Camera checkbox
						Fl_Button* buttonPlotCamera = new Fl_Button(485, 550, 120, 20, "Plot Keyframe");
						buttonPlotCamera->callback((Fl_Callback*)buttonPlotCameraCallback);

						// Clear Plot button
						Fl_Button* buttonClearPlot = new Fl_Button(610, 550, 120, 20, "Clear Keyframe");
						buttonClearPlot->callback((Fl_Callback*)buttonClearCameraCallback);
						
						// End this group
						cameraPane->end();
					}

					// Particle system buttons
					{ particlePane = new Fl_Group(255, 575, 540, 20, "Particle System");
						// Put particle system label inside
						particlePane->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
						particlePane->labelfont(FL_HELVETICA_BOLD);

						// Simulate button
						checkboxSimulate = new Fl_Check_Button(375, 575, 75, 20, "Simulate");
						checkboxSimulate->callback((Fl_Callback*)checkboxSimulateCallback);

						// Clear Particles button
						buttonClearParticles = new Fl_Button(455, 575, 50, 20, "Clear");
						buttonClearParticles->callback((Fl_Callback*)buttonClearParticlesCallback);

						// Particle "bake" indicator
						particleBakeIndicator = new IndicatorWindow(510, 575, 285, 20);
						// TODO: fix this:
						particleBakeIndicator->range(0, 20);
						particleBakeIndicator->deactivate();
						particlePane->resizable(particleBakeIndicator);

						// End this group
						particlePane->end();
					}

					// The playback controls
					{ Fl_Group* o = new Fl_Group(255, 600, 540, 20, "Playback");
						// Put particle system label inside
						o->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
						o->labelfont(FL_HELVETICA_BOLD);

						// Rewind button
						Fl_Button* rewind = new Fl_Button(335, 600, 20, 20, "@|<");
						rewind->labeltype(FL_SYMBOL_LABEL);
						rewind->callback((Fl_Callback*)buttonRewindCallback, this);

						// Play button
						buttonPlay = new Fl_Button(360, 600, 20, 20, "@>");
						buttonPlay->labeltype(FL_SYMBOL_LABEL);
						buttonPlay->callback((Fl_Callback*)buttonPlayCallback, this);
						buttonPlay->labelcolor(FL_GREEN);

						// Fast Forward button
						Fl_Button* fastForward = new Fl_Button(385, 600, 20, 20, "@>|");
						fastForward->labeltype(FL_SYMBOL_LABEL);
						fastForward->callback((Fl_Callback*)buttonFastForwardCallback, this);

						// Time slider
						sliderTime = new Fl_Value_Slider(455, 600, 340, 20, "Time:");
						sliderTime->type(FL_HORIZONTAL);
						sliderTime->align(FL_ALIGN_LEFT);
						sliderTime->callback((Fl_Callback*)sliderTimeCallback, this);
						sliderTime->bounds(0, 20);

						// Stretch the time slider on resize
						o->resizable(sliderTime);
						o->end();
					}
					curvePane->end();
				}
				w->resizable(m_modelerView);
				o->end();
				Fl_Group::current()->resizable(o);
			}
		}
		o->end();
	}
}

static char* argv[1];

void ModelerUserInterface::show() {
  m_controlsWindow->show();
  m_modelerView->show();
}

const char* ModelerUserInterface::fileDialog(Fl_Native_File_Chooser::Type dialogType, const char* filter, const char* title) {
  //------------------------------------------------------------------------
// This displays a modal file chooser with a native look-and-feel.
// The available dialog types are:
//   BROWSE_DIRECTORY       - Open a single directory
//   BROWSE_FILE            - Open a single file
//   BROWSE_MULTI_DIRECTORY - Open directories, allowing the user to
//                            select more than one at a time
//   BROWSE_MULTI_FILE      - Open files, allowing the user to select
//                            more than one at a time
//   BROWSE_SAVE_DIRECTORY  - Save a directory
//   BROWSE_SAVE_FILE       - Save a file
//
// The filter limits the displayed files. See cb_load_image for an example.
// title is optional, use NULL for the OS default title.
// The return value is the filepath.
//------------------------------------------------------------------------

	if(!m_nativeChooser)
		m_nativeChooser = new Fl_Native_File_Chooser(dialogType);
	else
		m_nativeChooser->type(dialogType);
	m_nativeChooser->filter(filter);
	m_nativeChooser->title(title);
	int ret = m_nativeChooser->show();
	if(ret == -1 ||	ret == 1) {
		//error or cancel respectively
		return NULL;
	}
	return m_nativeChooser->filename();
}

ModelerUserInterface::~ModelerUserInterface() {
	if(m_nativeChooser) delete m_nativeChooser;
	if (model) delete model;
	if (defaultCam) delete defaultCam;
}

void ModelerUserInterface::cb_Reload(Fl_Menu*, void*) {
	ModelerUserInterface::getInstance()->cb_Reload_i();
}

void ModelerUserInterface::cb_Reload_i() {
	// TODO: better resource error handling
	m_modelerView->retry();
}

void ModelerUserInterface::cb_GenShaders(Fl_Menu*, void*) {
	// Call instance method
	ModelerUserInterface::getInstance()->cb_GenShaders_i();
}

void ModelerUserInterface::cb_GenShaders_i() {
	// Generate shaders
	shaderSourceCode = new map<string, string>();
	m_modelerView->retry();
}

void ModelerUserInterface::cb_setFPS(Fl_Menu*, void*) {
	ModelerUserInterface::getInstance()->setFPS();
}

int ModelerUserInterface::getFPS() { return framesPerSecond; }

void ModelerUserInterface::setFPS(int fps) {
	framesPerSecond = fps;
}

void ModelerUserInterface::setFPS() {
	int testFPS;

	// Prompt for width.
	const char* szFPS = NULL;
	do {
		szFPS = fl_input("New Frames Per Second (1 ~ 50)", "");

		if (szFPS) {
			testFPS = atoi(szFPS);
		} else {
			return;
		}
	} while (szFPS && (testFPS < 1 || testFPS > 50));
	
	setFPS(testFPS);
}

void ModelerUserInterface::setModel(Model* model) {
	// Get rid of the old model.
	if (this->model) {
		delete this->model;
	}
	// Get rid of the default camera.
	if (defaultCam) {
		delete defaultCam;
	}

	// Set the model
	this->model = model;
	m_modelerView->setModel(model);

	// Set particle system
	ps = model->getParticleSystem();

	// Populate list with model items
	m_controlsTree->clear();
	pickGroupProperty(NULL);

	// Populate the list with group properties
    populateList(model->getProperties());

	// Select the first one
	if (m_controlsTree->first()) {
		m_controlsTree->select(m_controlsTree->first(), 1);
	}

	// Clear the curve list
	curvesTree->clear();

	// Populate the curve list
	populateCurveList(model->getProperties());

	// Create a default camera if the model doesn't provide one
	// TODO: move the other default camera things into #ANIMATOR guards
	if (!model->getCamera()) {
		defaultCam = new SimpleCameraModel("Default Camera");
		populateList(defaultCam->getProperties(), m_controlsTree->root());
		populateCurveList(defaultCam->getProperties(), curvesTree->root());
	}

	useCamera(true);
}

// TODO: populate the curve list once we actually have a list of curves.
void ModelerUserInterface::populateCurveList(Property* prop, Fl_Tree_Item* parent) {
	// Create a tree node for this property
	Fl_Tree_Item* item;
	if (parent == NULL) {
		// HACK: We have to add and remove a fake list item so that the tree
		// control will create a root node to put it under.
		curvesTree->remove(curvesTree->add("You shouldn't see this."));

		item = curvesTree->root();
		item->label(prop->getName());
	} else {
		item = curvesTree->add(parent, prop->getName());
	}
	item->labelfont(FL_HELVETICA_BOLD);
	if (GroupProperty* g = dynamic_cast<GroupProperty*>(prop)) {
		if (g->getCollapsed()) {
			item->close();
		}
	} else if (dynamic_cast<RGBProperty*>(prop)) {
		item->close();
	}

	// Examine the list of properties.
	PropertyList* controls = prop->getProperties();
	for (PropertyList::iterator iter = controls->begin();
		 iter != controls->end();
		 iter++) {
        // For now, only RangeProperty is supported by the Curve interface.
		// The RGBProperty is also indirectly supported because it can provide
	    // RangeProperties when its getProperties() method is called.
		// TODO: make this work with more property types (using a new
		// plotting widget?)
		if (RangeProperty* range = dynamic_cast<RangeProperty*>(*iter)) {
			// TODO: connect to Curve object instead of Property object.
			Fl_Tree_Item* childNode = curvesTree->add(item, range->getName());
			curveProps.push_back(range);
			childNode->user_data((void*)(curveProps.size() - 1));
			range->setCurveIndex(curveProps.size() - 1);
			graph->addCurve(range->getValue(), range->getMin(),
				range->getMax());
		} else if ((*iter)->getProperties() != NULL) {
			// Try to get a list of GroupProperties out of it.
			populateCurveList(*iter, item);
		}
	}
}

void ModelerUserInterface::populateList(GroupProperty* group, Fl_Tree_Item* parent) {
	// Create a tree node for this group.
	Fl_Tree_Item* item;
	if (parent == NULL) {
		// HACK: We have to add and remove a fake list item so that the tree
		// control will create a root node to put it under.
		m_controlsTree->remove(m_controlsTree->add("You shouldn't see this."));

		item = m_controlsTree->root();
		item->label(group->getName());
	} else {
		item = m_controlsTree->add(parent, group->getName());
	}
	item->user_data(group);

	// Examine the group's property list for group properties.
	PropertyList* controls = group->getProperties();
	for (PropertyList::iterator iter = controls->begin();
		 iter != controls->end();
		 iter++) {
	    // See if it's a GroupProperty by attempting to cast it
		GroupProperty* childGroup = dynamic_cast<GroupProperty*>(*iter);

		// If it is, add it to the list.
		if (childGroup) {
			ModelerUserInterface::populateList(childGroup, item);
		}
	}
}

void ModelerUserInterface::pickGroupProperty(GroupProperty* group) {
	// Remove the event listeners for old controls
	// TODO: we really need to have a PropertyEditor class that handles this
	// automatically...
	if (currentGroup) {
		PropertyList* props = currentGroup->getProperties();
		for (PropertyList::iterator iter = props->begin();
			 iter != props->end();
			 iter++)
		{
			if (RangeProperty* prop = dynamic_cast<RangeProperty*>(*iter)) {
				prop->unlisten((SignalListener)updateRangeSlider);
			} else if (RGBProperty* prop = dynamic_cast<RGBProperty*>(*iter)) {
				prop->unlisten((SignalListener)updateColorChooser);
			} else if (BooleanProperty* prop = dynamic_cast<BooleanProperty*>(*iter)) {
				prop->unlisten((SignalListener)updateCheckbox);
			} else if (ChoiceProperty* prop = dynamic_cast<ChoiceProperty*>(*iter)) {
				prop->unlisten((SignalListener)updateChoice);
			}
		}

		// Clear out the old controls
		m_controlsPack->clear();

		currentGroup = NULL;
	}

	// Reset the scrollbar
	m_controlsScroll->position(0, 0);

	// If there's no group, exit
	if (!group) {
		m_controlsScroll->redraw();
		return;
	}

	// Constants for slider dimensions
	const int packWidth = m_controlsPack->w();
	const int textHeight = 20;
	const int sliderHeight = 20;
	const int chooserHeight = 100;
	const int buttonHeight = 20;

	// Show them
	// For each control, add appropriate objects to the user interface
	currentGroup = group;
	PropertyList* props = group->getProperties();
	for (PropertyList::iterator iter = props->begin();
		 iter != props->end();
		 iter++)
    {
		// Ignore it if it's a group property (those belong in the tree).
		if (dynamic_cast<GroupProperty*>(*iter))
			continue;

		// And now we'll create a UI element for the property.
		// The big if-statement below uses dynamic_cast<PropertyType*>(ptr),
		// to see if a property has a given type.  dynamic_cast will
		// return 0 if ptr is not of type PropertyType.

		// Add a slider if the property is a RangeProperty
		if (RangeProperty* prop = dynamic_cast<RangeProperty*>(*iter)) {
			// Add the label
			Fl_Box *box = new Fl_Box(0, 0, packWidth, textHeight, (*iter)->getName());
			box->labelsize(14);
			box->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			box->box(FL_FLAT_BOX); // otherwise, Fl_Scroll messes up (ehsu)
			m_controlsPack->add(box);

			// Add the slider
			Fl_Value_Slider *slider = new Fl_Value_Slider(0, 0, packWidth, sliderHeight);
			slider->type(1);
			slider->range(prop->getMin(), prop->getMax());
			slider->step(prop->getStep());
			slider->value(prop->getValue());
			m_controlsPack->add(slider);

			// Use the step size to determine the number of decimal places
			// shown in the slider's label.
			if (prop->getStep() > 0) {
			  slider->precision((int)-log(prop->getStep()));
			}

			// Have the slider notify the program when it changes
			slider->callback((Fl_Callback*)SliderCallback, (void*) prop);

			

			// Have the property notify the slider when it changes
			prop->listen((SignalListener)updateRangeSlider, (void*) slider);

		// Add a color picker if the property is an RGB property
		} else if (RGBProperty* prop = dynamic_cast<RGBProperty*>(*iter)) {
			// Add the label
			Fl_Box *box = new Fl_Box(0, 0, packWidth, textHeight, (*iter)->getName());
			box->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			box->labelsize(14);
			box->box(FL_FLAT_BOX); // otherwise, Fl_Scroll messes up (ehsu)
			m_controlsPack->add(box);

			// Add a color chooser
			Fl_Color_Chooser* chooser = new Fl_Color_Chooser(0, 0, packWidth,
				chooserHeight);
			chooser->rgb(prop->getRed(), prop->getGreen(), prop->getBlue());
			m_controlsPack->add(chooser);

			// Have the chooser notify the program when it changes
			chooser->callback((Fl_Callback*)ColorPickerCallback, (void*) prop);

			// Remove any existing color chooser listeners on the property
			prop->unlisten((SignalListener)updateColorChooser);

			// Have the property notify the chooser when it changes
			prop->listen((SignalListener)updateColorChooser, (void*) chooser);

		// Add a checkbox if the property is a boolean property
		} else if (BooleanProperty* prop = dynamic_cast<BooleanProperty*>(*iter)) {
			// Add the checkbox -- no label needed!
			Fl_Check_Button* btn = new Fl_Check_Button(0, 0, packWidth, buttonHeight,
				prop->getName());
			btn->labelsize(14);
			btn->type(FL_TOGGLE_BUTTON);
			btn->value(prop->getValue());
			m_controlsPack->add(btn);

			// Have the button notify the program when it changes
			btn->callback((Fl_Callback*)ButtonCallback, (void*) prop);

			// Remove any existing color chooser listeners on the property
			prop->unlisten((SignalListener)updateCheckbox);

			// Have the property notify the chooser when it changes
			prop->listen((SignalListener)updateCheckbox, (void*) btn);

		// Add radio buttons if the property is a choice property
		} else if (ChoiceProperty* prop = dynamic_cast<ChoiceProperty*>(*iter)) {
			// Add the label
			Fl_Box *box = new Fl_Box(0, 0, packWidth, textHeight, (*iter)->getName());
			box->align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
			box->labelsize(14);
			box->box(FL_FLAT_BOX); // otherwise, Fl_Scroll messes up (ehsu)
			m_controlsPack->add(box);

			// Add a group
			Fl_Pack* pack = new Fl_Pack(0, 0, packWidth, buttonHeight);
			pack->type(Fl_Pack::VERTICAL);
			pack->box(FL_THIN_DOWN_FRAME);
			pack->user_data((void*) prop);

			// Add the radio buttons
			const char* choices = prop->getLabels();
			int start = 0, end = -1, index = 0;
			do {
				end++;
				if (choices[end] == 0 || choices[end] == '|') {
					string str(choices, start, end - start);
					Fl_Button* btn = new Fl_Round_Button(0, 0, packWidth, buttonHeight,
						prop->getName());
					btn->type(FL_RADIO_BUTTON);
					btn->copy_label(str.c_str());
					btn->value(prop->getValue() == index);

					// Have the button notify the program when it changes
					btn->callback((Fl_Callback*)ChoiceCallback, (void*)index);

					index++;
					start = end + 1;
				}
			} while (choices[end] != 0);

			pack->end();
			m_controlsPack->add(pack);

			// Remove any existing choce listeners on the property
			prop->unlisten((SignalListener)updateChoice);

			// Have the property update the choices when it changes
			prop->listen((SignalListener)updateChoice, (void*) pack);
		}
    }
	m_controlsScroll->redraw();
}

ModelerUserInterface* ModelerUserInterface::getInstance() {
	return instance;
}

void ModelerUserInterface::handleTimeoutInner() {
	// Only advance the timer if we're animating and we aren't still drawing the
	// last frame.  This prevents the timer from getting ahead of the drawing.
	if (animating && !drawing) {
		// Are we done?
		if (graph->currTime() + (1.0f / framesPerSecond) >= graph->endTime()) {
			stop();
			setTime(graph->endTime());
		} else {
			// Advance the scene
			setTime(graph->currTime() + (1.0f / framesPerSecond));
		}
	}

	// HACK: delete unnecessary controls
	if (!rendering && renderGroup) {
		m_controlsWindow->remove(*renderGroup);
		m_controlsWindow->init_sizes();
		delete renderGroup;
		renderGroup = NULL;
		m_controlsWindow->redraw();
	}

	// Call the timeout callback again.  Have no delay if we're rendering!
	Fl::add_timeout(rendering ? 0 : (1.0f / framesPerSecond), handleTimeout, this);
}

void ModelerUserInterface::handleTimeout(void* v) {
	ModelerUserInterface* ui = (ModelerUserInterface*)v;
	ui->handleTimeoutInner();
}

int ModelerUserInterface::run() {
	// Just tell FLTK to go for it.
	// TODO: Fl::visual might not be effective since it's called after
	// the FLTK window was created.
   	Fl::visual( FL_RGB | FL_DOUBLE );
	show();
	Fl::add_timeout(0, ModelerUserInterface::handleTimeout, this);

	// Transfer control to FLTK.
	return Fl::run();
}

void ModelerUserInterface::SliderCallback(Fl_Slider* slider, void* p) {
	RangeProperty* prop = (RangeProperty*) p;
	prop->setValue((float)slider->value());
	ModelerUserInterface::getInstance()->m_modelerView->redraw();
}

void ModelerUserInterface::ButtonCallback(Fl_Button* button, void* p) {
	BooleanProperty* prop = (BooleanProperty*) p;
	prop->setValue(button->value() ? true : false);
	ModelerUserInterface::getInstance()->m_modelerView->redraw();
}

void ModelerUserInterface::ColorPickerCallback(Fl_Color_Chooser* picker, void* p) {
	RGBProperty* prop = (RGBProperty*) p;
	prop->setColor((float)picker->r(), (float)picker->g(), (float)picker->b());
	ModelerUserInterface::getInstance()->m_modelerView->redraw();
}

void ModelerUserInterface::ChoiceCallback(Fl_Button* button, void* p) {
	ChoiceProperty* prop = (ChoiceProperty*) button->parent()->user_data();
	prop->setValue((int)p);
	ModelerUserInterface::getInstance()->m_modelerView->redraw();
}

void ModelerUserInterface::updateRangeSlider(RangeProperty* prop, Fl_Slider* slider, void* v) {
	slider->value(prop->getValue());
}

void ModelerUserInterface::updateCheckbox(BooleanProperty* prop, Fl_Button* checkbox, void* v) {
	checkbox->value(prop->getValue());
}

void ModelerUserInterface::updateColorChooser(RGBProperty* prop, Fl_Color_Chooser* chooser, void* v) {
	chooser->rgb(prop->getRed(), prop->getGreen(), prop->getBlue());
}

void ModelerUserInterface::updateChoice(ChoiceProperty* prop, Fl_Group* parent, void* v) {
	for (int i = 0; i < parent->children(); i++) {
		Fl_Button* button = (Fl_Button*)parent->child(i);
		button->value(((int)button->user_data() == prop->getValue()) ? 1 : 0);
	}
}

void ModelerUserInterface::TreeCallback(Fl_Tree* tree, void* p) {
	if (tree->callback_reason() == FL_TREE_REASON_SELECTED) {
		Fl_Tree_Item *item = tree->callback_item();

		ModelerUserInterface::getInstance()->pickGroupProperty(
		(GroupProperty*) item->user_data());
	}
}


void ModelerUserInterface::indicatorRangeMarkerRange(float fMin, float fMax) {
	particleBakeIndicator->rangeMarkerRange(fMin, fMax);
	particleBakeIndicator->rangeMarkerEnabled(true);
}

void ModelerUserInterface::pickCurve(int curve, bool selected) {
	graph->activateCurve(curve, selected);

	if (selected) { graph->zoomAll(); }
	graph->redraw();

	if (graph->getCurrCurve() > -1) {
		chooserCurveType->activate();
		checkboxWrap->activate();
		chooserCurveType->value(graph->currCurveType());
		checkboxWrap->value(graph->currCurveWrap());
		buttonZoomAll->activate();
	} else {
		chooserCurveType->deactivate();
		checkboxWrap->deactivate();
		buttonZoomAll->deactivate();
	}
}

/*
void ModelerUserInterface::setProperties() {
	Fl_Window* win = new Fl_Window(250, 400, "Movie Properties");
	{ Fl_Group* o = new Fl_Group(0, 20, 250, 80, "Dimensions");
		o->
	}
	win->end();
	win->set_modal();
	Fl::run();

}*/

float ModelerUserInterface::getTime() { return graph->currTime(); }

void ModelerUserInterface::setTime(float t) {
	// Exit if we're already at this time or already drawing.
	if (graph->currTime() == t || drawing) { return; }

	drawing = true;

	// Advance to the indicated time
	graph->currTime(t);

	// Loop over all properties, setting them to their curves' values.
	for (int i = 0; i < curveProps.size(); i++) {
		if (RangeProperty* prop =
			dynamic_cast<RangeProperty*>(curveProps[i])) {
			prop->setValue(graph->curve(i)->
				evaluateCurveAt(graph->currTime()));
		}
	}

	// Update time slider
	sliderTime->value(t);

	// Update particle simulation indicator
	particleBakeIndicator->floatingIndicator(t);

	if (simulating) {
		// update indicator window for particle simulation range
		if (ps != NULL) {
			float bakeStartTime = ps->getBakeStartTime();
			float bakeEndTime = ps->getBakeEndTime();
			if (bakeEndTime < 0.0f)
				bakeEndTime = t;
			indicatorRangeMarkerRange(bakeStartTime, bakeEndTime);
		}
	}

	// Redraw the views
	if (!rendering) {
		graph->redraw();	
		particleBakeIndicator->redraw();
	}
	m_modelerView->redraw();

	// If rendering a movie, update everything.
	if (rendering) {
		char strFrameNumber[10];
		sprintf(strFrameNumber, "%04d", renderFrame++);
		std::string strFileName = renderFileName;
		strFileName += strFrameNumber;
		strFileName += renderFileExt;

		try {
			saveFrame(strFileName.c_str(), renderFileType, renderJPEGQuality);
		} catch (...) {
			// Stop rendering the movie.
			stop();
		}

		renderProgress->value(t);
	}

	// HACK: Sync the simulation UI stuff.
	syncSimulate();

	drawing = false;
}

void ModelerUserInterface::stop() {
	animating = false;

	finishSavingMovie();
	
	// Show play icon on play button
	buttonPlay->label("@>");
	buttonPlay->labelcolor(FL_GREEN);
}

void ModelerUserInterface::play() {
	// Rewind first
	if (graph->currTime() >= graph->endTime()) {
		rewind();
	}

	// Start playing
	animating = true;

	// Show pause icon on play button
	buttonPlay->label("@||");
	buttonPlay->labelcolor(FL_YELLOW);
}

void ModelerUserInterface::playOrStop() {
	if (animating) {
		stop();
	} else {
		play();
	}
}

void ModelerUserInterface::rewind() {
	stop();
	setTime(0);
}

void ModelerUserInterface::fastForward() {
	stop();
	setTime(graph->endTime());
}

void ModelerUserInterface::snapModel() {
	int currCurve = graph->getCurrCurve();
	if (currCurve < 0) {
		return;
	}
	if (RangeProperty* prop =
		dynamic_cast<RangeProperty*>(curveProps[currCurve])) {
		prop->setValue(graph->curve(currCurve)->evaluateCurveAt(graph->currTime()));
		m_modelerView->redraw();
	}
}

void ModelerUserInterface::zoomAll() {
	graph->zoomAll();
	graph->redraw();
}

/***************************** CALLBACKS *********************************/

void ModelerUserInterface::CurveTreeCallback(Fl_Tree* tree, void* p) {
	Fl_Tree_Item *item = tree->callback_item();

	// Only process leaf nodes.
	if (item->children()) {
		return;
	}

	// Determine how to change selection state.
	switch (tree->callback_reason()) {
	case FL_TREE_REASON_SELECTED:
		ModelerUserInterface::getInstance()->pickCurve((int)item->user_data(), true);
		break;
	case FL_TREE_REASON_DESELECTED:
		ModelerUserInterface::getInstance()->pickCurve((int)item->user_data(), false);
		break;
	}
}

void ModelerUserInterface::chooserCurveTypeCallback(Fl_Choice* chooser, void* p) {
	GraphWidget* graph = ((ModelerUserInterface*)p)->graph;
	graph->currCurveType(chooser->value());
	graph->redraw();
}

void ModelerUserInterface::checkboxWrapCallback(Fl_Check_Button* button, void* p) {
	GraphWidget* graph = ((ModelerUserInterface*)p)->graph;
	graph->currCurveWrap(button->value() ? true : false);
	graph->redraw();
}

void ModelerUserInterface::buttonPlayCallback(Fl_Button* button, void* p) {
	((ModelerUserInterface*)p)->playOrStop();
}

void ModelerUserInterface::buttonRewindCallback(Fl_Button* button, void* p) {
	((ModelerUserInterface*)p)->rewind();
}

void ModelerUserInterface::buttonFastForwardCallback(Fl_Button* button, void* p) {
	((ModelerUserInterface*)p)->fastForward();
}

void ModelerUserInterface::graphCallback(GraphWidget* graph, void* p) {
	((ModelerUserInterface*)p)->snapModel();
}

void ModelerUserInterface::sliderTimeCallback(Fl_Value_Slider* slider, void* p) {
	((ModelerUserInterface*)p)->setTime(slider->value());
}

void ModelerUserInterface::buttonZoomAllCallback(Fl_Button* button, void* p) {
	((ModelerUserInterface*)p)->zoomAll();
}

void ModelerUserInterface::cb_setMovieLength(Fl_Menu_* o, void* v) {
	((ModelerUserInterface*)(o->parent()->user_data()))->setMovieLength();
}

void ModelerUserInterface::setMovieLength() {
	const char* strMovieLength;
	char lastLength[10];
	float testLength = 1;
	itoa(graph->endTime(), lastLength, 10);
	do {
		strMovieLength = fl_input(
			"How long, in seconds, would you like the movie to be? (1-600.0)",
			lastLength);
		if (!strMovieLength) {
			return;
		}
		testLength = (float)atof(strMovieLength);
		if (testLength >= 1 && testLength < 600) {
			break;
		}
	} while (true);

	setMovieLength(testLength);
}

void ModelerUserInterface::setMovieLength(float len) {
	graph->endTime(len);
	graph->redraw();
	
	sliderTime->range(0.0, len);
	sliderTime->redraw();
	
	particleBakeIndicator->range(0.0, len);
	particleBakeIndicator->redraw();
}

ParticleSystem* ModelerUserInterface::getParticleSystem() { return ps; }

void ModelerUserInterface::checkboxUseCameraCallback(Fl_Check_Button* checkbox, void* p) {
	ModelerUserInterface::getInstance()->
		useCamera(checkbox->value() ? true : false);
}

void ModelerUserInterface::buttonPlotCameraCallback(Fl_Button* button, void* p) {
	ModelerUserInterface::getInstance()->plotCamera();
}

void ModelerUserInterface::buttonClearCameraCallback(Fl_Button* button, void* p) {
	ModelerUserInterface::getInstance()->clearCamera();
}

void ModelerUserInterface::useCamera(bool use) {
	if (!model) { return; }
	if (use) {
		ICamera* target = model->getCamera();
		m_modelerView->m_camera->setTarget(target ? target : defaultCam);
	} else {
		m_modelerView->m_camera->setTarget(NULL);
	}
	if (use != (checkboxUseCamera->value() ? true : false)) {
		checkboxUseCamera->value(use ? true : false);
	}
	m_modelerView->redraw();
}

void ModelerUserInterface::plotCamera() {
	m_modelerView->m_camera->getTarget()->plotKeyframe(graph->currTime());
}

void ModelerUserInterface::clearCamera() {
	m_modelerView->m_camera->getTarget()->clearKeyframe(graph->currTime());
}

void ModelerUserInterface::setSimulate(bool sim) {
	simulating = sim;
	syncSimulate();
}

void ModelerUserInterface::checkboxSimulateCallback(
	Fl_Check_Button* checkbox, void* v) {
	ModelerUserInterface::getInstance()->setSimulate(
		checkbox->value() ? true : false);
}

void ModelerUserInterface::syncSimulate() {
	if (ps != NULL) {
		bool sim = ps->isSimulate();

		// stop simulation if we're at endTime
		const double TIME_EPSILON = 0.05;
		if (sim && (graph->currTime() >= (graph->endTime() - TIME_EPSILON))) {
			ps->stopSimulation(graph->currTime()); 
		}

		// check to see if we're simulating still
		sim = ps->isSimulate();
		if (simulating != sim) {
			// if the psystem is dirty,
			// we need to sync to it
			if (ps->isDirty()) {
				simulating = sim;
				// TODO: update UI controls?
				checkboxSimulate->value(sim);
			}
			// otherwise, we sync the psystem
			// to the ui
			else if (simulating) {
				ps->startSimulation(graph->currTime());
			} else {
				ps->stopSimulation(graph->currTime());
			}
		}
		ps->setDirty(false);
	}
}

void ModelerUserInterface::buttonClearParticlesCallback(Fl_Button* button, void* p) {
	ModelerUserInterface::getInstance()->clearParticles();
}

void ModelerUserInterface::clearParticles() {
	if (ps) {
		ps->clearBaked();
		particleBakeIndicator->rangeMarkerEnabled(false);
		particleBakeIndicator->redraw();
	}
}


