// The sample box model.  You should build a file
// very similar to this for when you make your model in
// order to plug in to the animator project.

#pragma warning (disable : 4305)
#pragma warning (disable : 4244)
#pragma warning (disable : 4786)
#pragma warning (disable : 4312)

#include "modelerview.h"
#include "modelerdraw.h"
#include "modelerui.h"
#include "vault.h"
#include "FL/glut.h"
#include "model.h"
#include "cameras.h"
#include "MyModel.h"

#include <cmath>

/** The scene, which includes the lights and models. */
class Scene : public Model {
protected:
///////////////////////////////// TEXTURES ////////////////////////////////////
    Texture2D texture;


///////////////////////////////// SHADERS /////////////////////////////////////
    ShaderProgram shader;

//////////////////////////////// PROPERTIES ///////////////////////////////////

    // Scene lights
    PointLight pointLight;
    DirectionalLight directionalLight;

    // My Model #1
    MyModel _myModel_1;

    // My Model #2
    MyModel _myModel_2;

    // My Model #3
    MyModel _myModel_3;

    // My Model #4
    MyModel _myModel_4;

public:
    ParticleSystem ps;

    /** Modeler calls this to get our particle system. */
    ParticleSystem* getParticleSystem() { return _myModel_1.getParticleSystem(); }

    /** Construct the scene */
    Scene() :
        // You have to call the parent class's constructor, to provide a
        // name for the model.
        Model("Scene"),

        _myModel_1(1, 0.68, 0.79, 5.0, 0.0, -2.0),

        _myModel_2(.2, 0.2, 0.2, 0.0, 0.0, 0.0),

        _myModel_3(.8, 0, 0, -5.0, 0.0, -2.0),

        _myModel_4(.8, 0.8, 0.8, 0.0, 0.0, -5.0),

        // Construct textures and shaders. 
        // They won't be loaded until the model is drawn for the first time.
        texture("floor_5.png"),
        shader("shader.vert", "shader.frag", NULL),

        // Call the constructors for the lights
        pointLight("Point Light", GL_LIGHT1, /**direction part**/ -5, 5, 5, /**diffuse part**/ 1.0, 0.5, 0.5, 
        /**specular part**/ 1.0, 0.5, 0.5, /**ambient part**/ .2f, 0.1, 0.1 /**attenuation part**/, 0.4, 0.7, 0),
        directionalLight("Directional Light", GL_LIGHT0, /**direction part**/ 5, 5, 5, /**diffuse part**/ 0.0f, 0.5, 0.5f, 
        /**specular part**/ 0.0f, 0.5f, 0.5f )

        // Now, call the constructors for each Property:
    {
        // If you have child Models, like the MobileLight model from model.h,
        // you can add their property groups, and they will appear in the list
        // in the top left corner of Modeler, under this model's entry:
        properties.add(pointLight.getProperties())
                  .add(directionalLight.getProperties());
        properties.add(_myModel_1.getProperties());
        properties.add(_myModel_2.getProperties());
        properties.add(_myModel_3.getProperties());
        properties.add(_myModel_4.getProperties());
        //properties.add(_myModel_2.getProperties());

        // Finally, add all the properties to this model's PropertyGroup.
    }

    /**
     * Modeler calls this method once an OpenGL context becomes available,
     * (which happens before the first time draw() is called), and whenever
     * you click the "Reload Textures and Shaders" menu option.
     *
     * Make sure you call the load() methods of each of your textures and
     * shaders here.
     */
    void load() {
        texture.load();
        shader.load();
    }

    /**
     * Modeler calls this method many times a second when the Animate
     * checkbox is checked.
     */
    void tick() {
        // You can put code here to increment animation counters for
        // extra credit.
    }


    /** Draw the scene. */
    void draw() {
        // The lights must be drawn FIRST, so the other scene elements
        // can get lit!
        pointLight.draw();
        directionalLight.draw();


        // Draw model men
        _myModel_1.draw();
        _myModel_2.draw();
        _myModel_3.draw();
        _myModel_4.draw();


        //
        // Draw the floor
        //
        ps.setGroundPlane(Vec3d(100,100,-2), Vec3d(0,-1,0));
        texture.use();

        //setDiffuseColor( .95, .95, .5);
        //setAmbientColor( .95, .95, .5 );

        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); // specify the texture coordinate
        glNormal3f(0, 1, 0); // specify the surface's normal at this vertex
        glVertex3f(-50, -2, -50); // both before its corresponding vertex

        glTexCoord2f(1, 1);
        glNormal3f(0, 1, 0);
        glVertex3f(50, -2, -50);

        glTexCoord2f(1, 0);
        glNormal3f(0, 1, 0);
        glVertex3f(50, -2, 50);

        glTexCoord2f(0, 0);
        glNormal3f(0, 1, 0);
        glVertex3f(-50, -2, 50);
        glEnd();

        // Stop applying textures to objects
        glBindTexture(GL_TEXTURE_2D, 0);

    }
};


/**
 * The program starts here.
 */
int main()
{
    // Tell the FLTK library to let us use OpenGL
    Fl::gl_visual(FL_RGB);

    init_load_curve_file();

    // Instantiate Modeler
    ModelerUserInterface ui;

    // Give Modeler your scene.
    // Modeler will free this pointer when it exits.
    ui.setModel(new Scene());


    // Run the application
    return ui.run();
}