#include <GLFW/glfw3.h>
extern GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

// center camera w positive Z
glm::vec3 position = glm::vec3(0, 0, 150);

float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
float initialFOV = 45.0f;

float speed = 3.0f;
float mouseSpeed = 0.005f;

void computeMatricesFromInputs(int controlType) {
	// only set once on first function call
	static double lastTime = glfwGetTime();

	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// reset cursor after getting position
	glfwSetCursorPos(window, width / 2, height / 2);

	// compute viewing angles based on input
	horizontalAngle += mouseSpeed * float(width / 2 - xpos);
	verticalAngle += mouseSpeed * float(height / 2 - ypos);
	// prevent player from looking over 90 degrees up/down
	float verticalClamp = 3.14 / 2;
	if (verticalAngle > verticalClamp) verticalAngle = verticalClamp;
	if (verticalAngle < -verticalClamp) verticalAngle = -verticalClamp;

	glm::vec3 forward(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// can't compute 'up' vector reliably, so we use 'right'
	glm::vec3 right(
		sin(horizontalAngle - 3.14 / 2.0f),
		0,
		cos(horizontalAngle - 3.14 / 2.0f)
	);

	// ...and find 'up' from there - perpendicular to right + forward
	glm::vec3 up = glm::cross(right, forward);

	float radius = position.z;

	switch (controlType) {
	case 1:
		// todo: not working
		position = glm::vec3(radius * cos(horizontalAngle), 0, radius * sin(verticalAngle));
	default:
		// wasd/spectator controls
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			position += forward * deltaTime * speed;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			position -= forward * deltaTime * speed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			position += right * deltaTime * speed;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			position -= right * deltaTime * speed;
		// up/down strafe
		glm::vec3 posY(0, 1.0f, 0);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			position += posY * deltaTime * speed;
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			position -= posY * deltaTime * speed;
		break;
	}

	ProjectionMatrix = glm::perspective(glm::radians(initialFOV), 4.0f / 3.0f, 0.1f, 500.0f);

	ViewMatrix = glm::lookAt(
		position,
		position + forward,
		up
	);

	lastTime = currentTime;
}