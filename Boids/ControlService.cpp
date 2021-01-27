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

void ControlService::ComputeMatricesFromInput() 
{
	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	glfwSetCursorPos(window, 1024 / 2, 768 / 2); // TODO: Fix this meme

	horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * float(768 / 2 - ypos);

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
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			AnimationPlaying = true;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			position += direction * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			position -= direction * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			position += right * deltaTime * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
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

void ControlService::initialize()
{

}