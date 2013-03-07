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

    //ShaderProgram part3Shader;

    //////////////////////////////// PROPERTIES ///////////////////////////////////
    // Switches for spheres
    BooleanProperty useTexture;
    // Wwitch for showing/hiding reference unit sphere
    BooleanProperty showReferenceUnitSphere;

    // Lets you pick what shapes to use for the default model!
    ChoiceProperty shapeChoice;

    BooleanProperty useShader;

    //BooleanProperty usePart3Shader;

    // Some slider properties
    RangeProperty rotateX, rotateY;

    // Diffuse color picker
    RGBProperty diffuse;


    // Scene lights
    PointLight pointLight;
    DirectionalLight directionalLight;
    //SpotLight spotLight;

    // My Model
    MyModel _myModel;

public:

    /** Construct the scene */
    Scene() :
        // You have to call the parent class's constructor, to provide a
        // name for the model.
        Model("Scene"),

        // Construct textures and shaders. 
        // They won't be loaded until the model is drawn for the first time.
        texture("checkers.png"),
        shader("shader.vert", "shader.frag", NULL),
        //part3Shader("shader_part3.vert", "shader_part3.frag", NULL),

        // Call the constructors for the lights
        pointLight("Point Light", GL_LIGHT1, /**direction part**/ -5, 5, 5, /**diffuse part**/ 1.0, 0.5, 0.5, 
        /**specular part**/ 1.0, 0.5, 0.5, /**ambient part**/ .2f, 0.1, 0.1 /**attenuation part**/, 0.4, 0.7, 0),        
        directionalLight("Directional Light", GL_LIGHT0, /**direction part**/ 5, 5, 5, /**diffuse part**/ 0.0f, 0.5, 0.5f, 
        /**specular part**/ 0.0f, 0.5f, 0.5f ),
        //spotLight("Spot Light", GL_LIGHT2, /**direction part**/ 0, 5, 5, /**diffuse part**/ 1.0, 1.0, 1.0, 
        ///**specular part**/ 1.0, 1.0, 1.0, /**ambient part**/ .2f, 0.1, 0.1 /**attenuation part**/, 0.4, 0.7, 0),

        // Now, call the constructors for each Property:
        useTexture("Use Checkered Texture", false),
        showReferenceUnitSphere("Show Reference Unit Sphere", false),
        shapeChoice("Model Shape:", "Sphere|Cube|Cylinder|Torus|Icosahedron|Teapot|Revolution|My Model", 0),
        useShader("Use My Shader", false),
        //usePart3Shader("Use Part3 Shader", false),
        rotateX("Rotate X", -180, 180, 0, 1),
        rotateY("Rotate Y", -180, 180, 0, 1),
        diffuse("Diffuse Color", 1.0, 0.7, .4)
    {
        // If you have child Models, like the MobileLight model from model.h,
        // you can add their property groups, and they will appear in the list
        // in the top left corner of Modeler, under this model's entry:
        properties.add(pointLight.getProperties())
            .add(directionalLight.getProperties())
            //.add(spotLight.getProperties())
            .add(_myModel.getProperties());

        // Finally, add all the properties to this model's PropertyGroup.
        properties.add(&useTexture)
            .add(&showReferenceUnitSphere)
            .add(&shapeChoice)
            .add(&useShader)
            //.add(&usePart3Shader)
            .add(&rotateX)
            .add(&rotateY)
            .add(&diffuse)
            ;
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
        //part3Shader.load();
        _myModel.load();
    }

    /**
    * Modeler calls this method many times a second when the Animate
    * checkbox is checked.
    */
    void tick() {
        // You can put code here to increment animation counters for
        // extra credit.
    }

    /** Draw the model. */
    void drawModel() {

        int divisions=16; 		
        switch(ModelerDrawState::Instance()->m_quality)
        {
        case HIGH: 
            divisions = 64; break;
        case MEDIUM: 
            divisions = 20; break;
        case LOW:
            divisions = 12; break;
        case POOR:
            divisions = 8; break;
        }

        // Remember our current modelview matrix, which is in world space.
        glPushMatrix();

        // Rotate around the X-axis
        glRotatef(rotateX.getValue(), 1, 0, 0);

        // Rotate around the Y-axis
        glRotatef(rotateY.getValue(), 0, 1, 0);


        // Draw some object at the origin.
        switch (shapeChoice.getValue()) {
        case 0: // sphere
            glRotatef(-90.f,1.f,0.f,0.f);
            drawSphere(1);
            glRotatef(90.f,1.f,0.f,0.f);
            break;
        case 1: // box
            glRotatef(-45.f,0.f,1.f,0.f);
            glutSolidCube(2);
            glRotatef(45.f,0.f,1.f,0.f);
            break;
        case 2: // cylinder
            glRotatef(90.f,1.f,0.f,0.f);
            drawCylinder(1.8,1.0,1.8);
            glRotatef(-90.f,1.f,0.f,0.f);
            break;
        case 3: // torus
            glutSolidTorus(.5,1,divisions,divisions);
            break;
        case 4: // icosahedron
            glutSolidIcosahedron();
            break;
        case 5: // teapot
            glutSolidTeapot(1);
            break;
        case 6:
            drawRevolution(1);
            break;
        case 7:
            _myModel.draw();
            break;
        }

        if ( showReferenceUnitSphere.getValue() ) {
            // Now go 1 units in the +y direction,
            // and 1 units in the NEGATIVE x direction.
            glTranslated(-2, -1, 0);
            // Now go 1 units in the +y direction,
            // and 1 units in the NEGATIVE x direction.
            glRotatef(-90.f,1.f,0.f,0.f);
            drawSphere(.5);
            glRotatef(90.f,1.f,0.f,0.f);
            glPopMatrix();
        }

        // Go back to "world space"
        glPopMatrix();

        // Draw the floor If not drawing MyModel.  Here's a basic example of texture mapping.
        if(shapeChoice.getValue() != 7)
        {
            glBegin(GL_QUADS);
            glTexCoord2f(0, 1); // specify the texture coordinate
            glNormal3f(0, 1, 0); // specify the surface's normal at this vertex
            glVertex3f(-10, -2, -10); // both before its corresponding vertex

            glTexCoord2f(1, 1);
            glNormal3f(0, 1, 0);
            glVertex3f(10, -2, -10);

            glTexCoord2f(1, 0);
            glNormal3f(0, 1, 0);
            glVertex3f(10, -2, 10);

            glTexCoord2f(0, 0);
            glNormal3f(0, 1, 0);
            glVertex3f(-10, -2, 10);
            glEnd();
        }
    }

    /** Draw the scene. */
    void draw() {
        // The lights must be drawn FIRST, so the other scene elements
        // can get lit!
        pointLight.draw();
        //spotLight.draw();
        directionalLight.draw();

        setDiffuseColor(
            diffuse.getRed(),
            diffuse.getGreen(),
            diffuse.getBlue());

        setSpecularColor(1,1,1);
        setShininess(pow(2.71828183f, 3));

        // Use the texture if desired.
        if (useTexture.getValue()) {
            texture.use();
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        //if (usePart3Shader.getValue()) {
        //    part3Shader.use();
        //}

        if (useShader.getValue()) {
            // Use the shader if desired.
            shader.use();
        }

        // Call a class method that draws our model.
        drawModel();
        // Stop applying shaders to objects.
        // This if-statement makes sure that glUseProgram is not a null
        // function pointer (which it will be if GLEW couldn't initialize).
        if (glUseProgram) {
            glUseProgram(0);
        }

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