#include "ControlService.h"

glm::vec3 ControlService::position = glm::vec3(0, 0, 5);
float ControlService::horizontalAngle = 3.00f;
float ControlService::verticalAngle = -0.5f;
float ControlService::initialFoV = 45.0f;
float ControlService::speed = 3.0f;
float ControlService::mouseSpeed = 0.005f;
glm::mat4 ControlService::ViewMatrix = glm::mat4(0.0f);
glm::mat4 ControlService::ProjectionMatrix = glm::mat4(0.0f);
GLFWwindow* ControlService::window = nullptr;
bool AnimationPlaying = false;
int counter = 0;
ControlService::windowSize size = ControlService::windowSize(1600, 1200);

void ControlService::ComputeMatricesFromInput() 
{
	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	glfwSetCursorPos(window, size.width / 2, size.height / 2);

	horizontalAngle += mouseSpeed * float(size.width / 2 - xpos);
	verticalAngle += mouseSpeed * float(size.height / 2 - ypos);

	//Direction Vector (Polar to Cartesian conversion)
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle), 
		cos(verticalAngle) * cos(horizontalAngle));

	//Right Vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	//Up Vector
	glm::vec3 up = glm::cross(right, direction);


	//Movement Handling
	if (!AnimationPlaying)
	{
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			AnimationPlaying = true;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			position += direction * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			position -= direction * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			position += right * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			position -= right * deltaTime * speed;
		}		
	}
	else
	{
		position += right * deltaTime * speed;
		counter++;
		if (counter >= 500)
		{
			AnimationPlaying = false;
			counter = 0;
		}
	}

	float FoV = initialFoV; // FoV changable via Callback

	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);

	ViewMatrix = glm::lookAt(
		position,
		position + direction,
		up
	);
	lastTime = currentTime;

}

float ControlService::getDistanceFromOrigin() {
	return glm::length(position);
}

bool ControlService::getTransparent() {
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		return true;
	}
	return false;
}

ControlService::windowSize ControlService::getWindowSize() {
	return size;
}

void ControlService::initialize(){

}