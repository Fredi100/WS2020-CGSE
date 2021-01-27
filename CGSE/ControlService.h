#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class ControlService
{
private:
	ControlService() = default;
	~ControlService() = default;
	ControlService(const ControlService&) = delete;
	ControlService& operator=(const ControlService&) = delete;

	static glm::mat4 ViewMatrix;
	static glm::mat4 ProjectionMatrix;
	static glm::vec3 position;
	static float horizontalAngle;
	static float verticalAngle;
	static float initialFoV;
	static float speed;
	static float mouseSpeed;

	static GLFWwindow* window;


public:
	static ControlService& getInstance() {
		static ControlService instance;
		return instance;
	}
	static glm::mat4 getViewMatrix() {
		return ViewMatrix;
	}

	static glm::mat4 getProjectionMatrix() {
		return ProjectionMatrix;
	}

	static void setWindow(GLFWwindow* window)
	{
		ControlService::window = window;
	}

	typedef struct windowSize {
		int width;
		int height;
		windowSize(int width, int height) : width(width), height(height) {};
	} windowSize;

	static windowSize getWindowSize();

	static float getDistanceFromOrigin();
	static bool getTransparent();
	static void ComputeMatricesFromInput();
	static void initialize();

};

