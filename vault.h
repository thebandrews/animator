/**
 * The Vault is for loading and storing resources that you only need one
 * copy of, but that you use in many places in your model:
 *
 *  * textures
 *
 *  * vertex shaders
 *
 *  * fragment shaders
 *
 * For this project, you'll extend Vault to create a vault for your own model.
 */
#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <exception>
#include "GL/glew.h"
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include "imageio.h"
#include "rijndael.h"

/**
 * Tracks the shader key.
 */
extern const char* SHADER_KEY;

/** Shader filenames and data */
extern const char* SHADER_FILENAMES[];
extern const char* SHADER_DATA[];
extern int SHADER_COUNT;

/**
 * HACK: When this pointer is not NULL, shader source code
 * from loaded shaders is stored inside it.  This pointer is not NULL
 * whenever you generate shaders.cpp.
 */
extern std::map<std::string, std::string>* shaderSourceCode;

void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize);
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize);

const char* encrypt(const char* unhexed, const char* key);
const char* decrypt(const char* hexed, const char* key);

/**
 * Stores a shader's OpenGL ID and its source code filename.
 */
class ShaderItem {
public:
	GLuint id;
	const char* source_code;
	GLuint type;

	ShaderItem(const char* _source_code, GLuint _type);

	void load(GLuint program);
	void unload(GLuint program);
};

/** Stores a list of shaders. */
typedef std::vector<ShaderItem> ShaderList;

/**
 * A shader program, which includes vertex and fragment shaders.
 */
class ShaderProgram {
protected:
	/** List of OpenGL shader ID's and their filenames. */
	ShaderList shaders;

	/** OpenGL ID for the shader program. */
	GLuint program;

public:
	/**
	 * Creates a shader program, which loads and compiles shaders from the
	 * given source code.
	 *
	 * @param vertexShader   Filename of your vertex shader, or NULL if you
	 *                       have no vertex shader.
	 *
	 * @param fragmentShader Filename of your fragment shader, or NULL if you
	 *                       have no fragment shader.
	 *
	 * @param geometryShader Filename of your geometry shader, or NULL if you
	 *                       don't have one.  This parameter is OPTIONAL.
	 *
	 * @throw an exception if the shader can't be created.
	 *        This exception will be handled by Modeler when it tries to create
	 *        your shader, and will report things like compiler errors and file
	 *        reading errors.
	 */
	ShaderProgram(const char* vertexShader,
				  const char* fragmentShader,
				  const char* geometryShader = NULL);

	/** Makes sure unload() gets called. */
	~ShaderProgram();

	/** Returns the shader program's ID. */
	const GLuint getID() const;

	/** Tells OpenGL to use this texture. */
	void use() const;

	/** Load the shader */
	void load();

	/** Detaches and deletes the shader program and its shaders. */
	void unload();
};

/**
 * Texture loader
 */
class Texture2D {
protected:
	GLuint texture, envMode;

	const char* image;
public:
	Texture2D(const char* _image,
			  const GLuint _envMode = GL_REPLACE);

	/** Returns the texture ID. */
	const GLuint getID() const;

	/** Tells OpenGL to use this texture. */
	void use() const;

	/** Makes sure unload() gets called. */
	~Texture2D();

	/** Load the texture */
	void load();

	/** Unload the texture, freeing its OpenGL resources. */
	void unload();
};

/**
 * Reads a file into a string (useful for reading shader code into memory).
 */
const char* readFile(const char* const file);