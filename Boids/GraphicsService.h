#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ControlService.h"
#include "ObjectLoader.h"


class GraphicsService {
private:
	GraphicsService() = default;
	~GraphicsService() = default;
	GraphicsService(const GraphicsService&) = delete;
	GraphicsService& operator=(const GraphicsService&) = delete;

	GLuint programID;
	GLuint matrixID;

	GLFWwindow* window = nullptr;
	static ControlService* cService;

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void error_callback(int error, const char* description) {
		std::cerr << "Error: " << description << std::endl;
	}

	GLuint loadShaders(const char* vertex_file_path, const char* fragment_file_path);


public:
	static GraphicsService& getInstance() {
		static GraphicsService instance;
		return instance;
	}
	static GLuint loadBMP_custom(const char* imagepath);
	static GLuint loadDDS(const char* imagepath);

	static void setControlService(ControlService& service)
	{
		cService = &service;
	}

	void initialize();
	void run();

	void rotateMesh();

};

