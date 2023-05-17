#include "Window.h"

Window::Window()
{
	// Initialize the window dimensions
	width = 800;
	height = 600;

	// Initialize xChange and yChange
	xChange = 0.0f;
	yChange = 0.0f;

	// Initialize the array of keys (booleans) as false
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = false;
	}
}

Window::Window(GLint windowWitdh, GLint windowHeight)
{
	width = windowWitdh;
	height = windowHeight;

	xChange = 0.0f;
	yChange = 0.0f;

	// Initialize the array of keys (booleans) as false
	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = false;
	}
}

int Window::Initialize()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		printf("GLFW Initialization failed!");
		glfwTerminate();
		return 1;
	}

	// Setup GLFW window properties
	// OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// Core profile = No Backwards Compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Allow forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the window
	mainWindow = glfwCreateWindow(width, height, "Test Window", NULL, NULL);

	// Check if the window is created
	if (!mainWindow)
	{
		printf("GLFW window creation failed!");
		glfwTerminate();
		return 1;
	}

	// Get Buffer size information
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

	// Handle Keyboard + Mouse Input
	createCallbacks();
	glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  // Lock the mouse into place

	// Allow modern extension features (experimental features)
	glewExperimental = GL_TRUE;

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		printf("GLEW initialization failed!");
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);

	// Setup viewport size
	glViewport(0, 0, bufferWidth, bufferHeight);

	glfwSetWindowUserPointer(mainWindow, this);
}

void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, handleKeys);
	glfwSetCursorPosCallback(mainWindow, handleMouse);
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

void Window::handleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	// Get  access to the window, in order to get its keys.
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	// Close the window (ESCAPE)
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			// Set the key to pressed
			theWindow->keys[key] = true;
			//printf("pressed: %d\n", key);
		}
		else if (action == GLFW_RELEASE)
		{
			// Set the key to released
			theWindow->keys[key] = false;
			//printf("released: %d\n", key);
		}
	}
}

void Window::handleMouse(GLFWwindow* window, double xPos, double yPos)
{
	// Get  access to the window, in order to get access to its mouse
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	// Get the first coordinates
	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = xPos;
		theWindow->lastY = yPos;
		theWindow->mouseFirstMoved = false;
	}

	// Get the movement of the mouse
	theWindow->xChange = xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - yPos;

	// Update the last coordinates
	theWindow->lastX = xPos;
	theWindow->lastY = yPos;

	//printf("x: %.6f, y: %.6f\n", theWindow->xChange, theWindow->yChange);
}



Window::~Window()
{
	glfwDestroyWindow(mainWindow);
	glfwTerminate();
}


