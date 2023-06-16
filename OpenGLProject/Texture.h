#pragma once

#include <GL\glew.h>

#include "CommonValues.h"

class Texture
{
public:
	Texture();
	Texture(const char* fileLoc);

	bool LoadTexture();
	bool LoadTextureA();
	bool LoadColorTexture(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);

	void UseTexture();
	void UseTextureBlending();
	void ClearTexture();

	~Texture();

private:
	GLuint textureID;
	int width, height, bitDepth;

	const char* fileLocation;
};

