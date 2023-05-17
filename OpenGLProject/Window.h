#pragma once

#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWitdh, GLint windowHeight);

	int Initialize();

	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }

	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }

	bool* getKeys() { return keys; }
	GLfloat getXChange();
	GLfloat getYChange();


	void swapBuffers() { glfwSwapBuffers(mainWindow); }

	~Window();

private:
	GLFWwindow* mainWindow;

	GLint width, height;
	GLint bufferWidth, bufferHeight;
	
	// Declare Keyboard variables
	bool keys[1024];

	// Declare Mouse variables
	GLfloat lastX, lastY;
	GLfloat xChange, yChange;
	bool mouseFirstMoved;

	// Function to bind the callback functions (keyboard, mouse, etc.)
	void createCallbacks();

	// Function to handle keyboard events
	static void handleKeys(GLFWwindow* window, int key, int code, int action, int mode);

	// Function to handle mouse events
	static void handleMouse(GLFWwindow* window, double xPos, double yPos);
};

