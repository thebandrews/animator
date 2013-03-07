#include "vault.h"

#include <sys/stat.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <FL/fl_ask.H>
#include <FL/glu.h>

using namespace std;

map<string, string>* shaderSourceCode = NULL;

//Function to convert unsigned char to string of length 2
void Char2Hex(unsigned char ch, char* szHex)
{
	unsigned char byte[2];
	byte[0] = ch/16;
	byte[1] = ch%16;
	for(int i=0; i<2; i++)
	{
		if(byte[i] >= 0 && byte[i] <= 9)
			szHex[i] = '0' + byte[i];
		else
			szHex[i] = 'A' + byte[i] - 10;
	}
	szHex[2] = 0;
}

//Function to convert string of length 2 to unsigned char
void Hex2Char(char const* szHex, unsigned char& rch)
{
	rch = 0;
	for(int i=0; i<2; i++)
	{
		if(*(szHex + i) >='0' && *(szHex + i) <= '9')
			rch = (rch << 4) + (*(szHex + i) - '0');
		else if(*(szHex + i) >='A' && *(szHex + i) <= 'F')
			rch = (rch << 4) + (*(szHex + i) - 'A' + 10);
		else
			break;
	}
}    

//Function to convert string of unsigned chars to string of chars
void CharStr2HexStr(unsigned char const* pucCharStr, char* pszHexStr, int iSize)
{
	int i;
	char szHex[3];
	pszHexStr[0] = 0;
	for(i=0; i<iSize; i++)
	{
		Char2Hex(pucCharStr[i], szHex);
		strcat(pszHexStr, szHex);
	}
}

//Function to convert string of chars to string of unsigned chars
void HexStr2CharStr(char const* pszHexStr, unsigned char* pucCharStr, int iSize)
{
	int i;
	unsigned char ch;
	for(i=0; i<iSize; i++)
	{
		Hex2Char(pszHexStr+2*i, ch);
		pucCharStr[i] = ch;
	}
}

const char* encrypt(const char* unhexed, const char* key) {
	int size = strlen(unhexed);
	int spaces = 0;
	if (size % 16) {
		spaces = 16 - (size % 16);
	}
	size += spaces;

	char* buffer = new char[size+1];
	// TODO: memory check

	// Add spaces to the end to make size divisible by block size
	for (; spaces > 0; spaces--) {
		buffer[size-spaces] = ' ';
	}

	// Create reader
	CRijndael reader;
	reader.MakeKey(key, CRijndael::sm_chain0, 16, 16);
	reader.Encrypt(unhexed, buffer, size, CRijndael::CFB);

	// Convert to hex
	char* result = new char[size*2+1];
	CharStr2HexStr((unsigned char*)buffer, result, size);
	delete [] buffer;

	return result;
}

const char* decrypt(const char* hexed, const char* key) {
	int size = strlen(hexed) / 2;
	if (size % 16 != 0) {
		fl_alert("Encrypted file is corrupt.");
		throw false;
	}
	char* buffer = new char[size+1];
	HexStr2CharStr(hexed, (unsigned char*)buffer, size);
	CRijndael reader;
	reader.MakeKey(key, CRijndael::sm_chain0, 16, 16);
	char* buffer2 = new char[size+1];
	buffer2[size] = 0;
	if (!buffer2) {
		fl_alert("Not enough memory to load shader source code.");
		throw false;
	}
	reader.Decrypt(buffer, buffer2, size, CRijndael::CFB);
	return buffer2;
}

const char* readFile(const char* file) {
	assert(file != NULL);

	ifstream codeFile(file, ios::binary);
	if (codeFile.fail()) {
		fl_alert("Failed to open file %s", file);
		throw false;
	}
	codeFile.seekg(0, ios_base::end);
	int size = (int)codeFile.tellg();
	codeFile.seekg(0, ios_base::beg);

	// Allocate space for buffer
	char* buffer = new char[size+1];
	if (!buffer) {
		fl_alert("Not enough memory to load shader source code.");
		throw false;
	}

	// Read in the file
	codeFile.read(buffer, size);
	buffer[size] = 0;
	codeFile.close();

	return buffer;
}

ShaderProgram::ShaderProgram(const char* vertexShader,
						     const char* fragmentShader,
							 const char* geometryShader) : program(0) {
	// Add the shaders
	if (vertexShader) {
		shaders.push_back(ShaderItem(vertexShader, GL_VERTEX_SHADER));
	}
	if (fragmentShader) {
		shaders.push_back(ShaderItem(fragmentShader, GL_FRAGMENT_SHADER));
	}
	if (geometryShader) {
		shaders.push_back(ShaderItem(geometryShader, GL_GEOMETRY_SHADER_EXT));
	}
}

ShaderItem::ShaderItem(const char* _source_code, GLuint _type) :
	source_code(_source_code), type(_type) {}

void ShaderItem::load(GLuint program) {
	// Create the shader object
	id = glCreateShader(type);
	if (!id) {
		fl_alert("Failed to create a shader for %s", source_code);
		throw false;
	}

	try {

		// Stores file data
		const char* buffer = NULL;

		// Read file if file exists
		struct stat fileInfo;
		if (!stat(source_code, &fileInfo)) {
			buffer = readFile(source_code);
		} else if (SHADER_COUNT) {
			// Assume the file couldn't be found, so try to pull it out of the encrypted
			// shaders cache.
			// TODO: better error handling
			for (int i = 0; i < SHADER_COUNT; i++) {
				if (!strcmp(SHADER_FILENAMES[i], source_code)) {
					buffer = decrypt(SHADER_DATA[i], SHADER_KEY);
					break;
				}
			}
		}

		if (!buffer) {
			fl_alert("Could not read source code for shader %s", source_code);
			throw false;
		}

		// Add shader to cache if we're writing shaders.cpp
		if (shaderSourceCode) {
			(*shaderSourceCode)[source_code] = buffer;
		}

		// Load source code for it
		glShaderSource(id, 1, (const GLchar**) &buffer, NULL);
		delete [] buffer;

		// Attach to the program object
		glAttachShader(program, id);

		// Compile the current shader
		glCompileShader(id);

		// Verify that the shader compiled, and return a compile error if not.
		GLint status;
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			GLint logLength = 0, maxLength;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
			char* log = new char[maxLength];
			glGetShaderInfoLog(id, maxLength, &logLength, log);
			
			if (logLength > 0) {
				fl_alert("Failed to compile shader %s:\n%s", source_code, log);
			} else {
				fl_alert("Failed to compile shader %s.", source_code);
			}

			delete log;
			throw false;
		}
	} catch (...) {
		// Make sure we clean up
		unload(program);
		throw;
	}
}

void ShaderItem::unload(GLuint program) {
	if (!id) { return; }
	glDetachShader(program, id);
	glDeleteShader(id);
	id = 0;
}

void ShaderProgram::load() {
	// If we have a program, clear it out.
	if (program) { unload(); }

	// Make sure we clean up if an exception is thrown.
	try {
		// Create the program object
		program = glCreateProgram();
		if (!program) {
			// TODO: throw better exception!
			throw false;
		}

		// Loop over each shader, loading and compiling it.
		for (ShaderList::iterator iter = shaders.begin();
			 iter != shaders.end();
			 iter++) {
			iter->load(program);
		}

		// Link shaders
		glLinkProgram(program);

		// Check for linker errors
		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status != GL_TRUE) {
			GLint logLength = 0, maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
			char* log = new char[maxLength];
			glGetProgramInfoLog(program, maxLength, &logLength, log);
		
			if (logLength > 0) {
				fl_alert("Failed to link the shader:\n%s", log);
			} else {
				fl_alert("Failed to link the shader - no explanation.");
			}

			delete log;
			throw false;
		}
	} catch (...) {
		// Destroy the program and all attached shaders
		unload();

		// Rethrow the exception
		throw;
	}
}

void ShaderProgram::unload() {
	if (!program) { return; }

	// Detach and delete shaders
	for (ShaderList::iterator iter = shaders.begin();
		 iter != shaders.end();
		 iter++) {
		iter->unload(program);
	}

	// Delete the shader program
	glDeleteProgram(program);
	program = 0;
}

ShaderProgram::~ShaderProgram() {
	if (program) unload();
}

const GLuint ShaderProgram::getID() const {	return program; }
void ShaderProgram::use() const { glUseProgram(program); }

Texture2D::Texture2D(const char* _image, const GLuint _envMode) :
	image(_image), envMode(_envMode) {
	// Verify parameters
	assert(image != NULL);
	assert(envMode == GL_BLEND ||
		envMode == GL_MODULATE ||
		envMode == GL_REPLACE ||
		envMode == GL_DECAL);
}

void Texture2D::load() {
	// Unload texture if already loaded
	if (texture) { unload(); }

	// Load bitmap
	int width, height;
	unsigned char* pixels = loadImage(image, width, height);
	if (!pixels) {
		fl_alert("Failed to load texture %s.", image);
		throw false;
	}

	// Get an unused texture ID
	glGenTextures(1, &texture);
	
	// Tell OpenGL we want to work with this texture.
	glBindTexture(GL_TEXTURE_2D, texture);

	// Use a linear magnification filter (for enlarging the texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

	// Use a linear minification filter (for shrinking the texture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

    // Use the nearest mipmap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	// Repeat the texture horizontally (wrap the S-coordinate)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	// Repeat the texture vertically (wrap the T-coordinate)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// The last parameter tells how to blend the environment with the texture.
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envMode);

	// Tell OpenGL how the pixels are packed
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glPixelStorei( GL_UNPACK_ROW_LENGTH, width );
	
	// Send the pixels to OpenGL, and tell it to make mipmaps too!
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	// Delete the texture pixels
	delete [] pixels;

	// Unbind the texture to prevent it from appearing on shapes!
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::unload() {
	glDeleteTextures(1, &texture);
	texture = 0;
}

const GLuint Texture2D::getID() const { return texture; }
void Texture2D::use() const {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
}
Texture2D::~Texture2D() { if (texture) { unload(); } }