#include <GLFW/glfw3.h>
extern GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

#include <iostream>

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

int cwidth, cheight, cx, cy, cz;
glm::vec3 position = glm::vec3(25, 125, 0);
glm::vec3 oldposition;

void initControls(int width, int height, int x, int y, int z) {
	cwidth = width;
	cheight = height;
	cx = x;
	cy = y;
	cz = z;
	position = glm::vec3(cx/4 - 115, cy/2, cz/2);
	oldposition = glm::vec3((cx / 2) - 10, (cy / 2) + 10, (cz / 2));
}

float horizontalAngle = 0.5*3.14f;
float verticalAngle = 0.0f;
float initialFoV = 45.0f;

float speed = 10.0f;
float mouseSpeed = 0.005f;
int lookenable = 0;
int mousepress = 0;
int mousefree = 0;
int isfoc = 1;


void togglePosition() {
	glm::vec3 temp = position;
	position = oldposition;
	oldposition = temp;
}

void isFocus(int i) {
	isfoc = i;
}

void isMousePress(int i) {
	mousepress = i;
}

void lookEnable() {
	if (lookenable)
		lookenable = 0;
	else
		lookenable = 1;
}

void toggleFreeMouse() {
	if (mousefree)
		mousefree = 0;
	else
		mousefree = 1;
}

void resetCamera() {
	position = glm::vec3(cx / 4 - 115, cy / 2, cz / 2);
	horizontalAngle = 0.5*3.14f;
	verticalAngle = 0.0f;
}


void computeMatricesFromInputs(){

	static double lastTime = glfwGetTime();

	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (!mousefree) {
		glfwSetCursorPos(window, cwidth / 2, cheight / 2);
	}

	if (lookenable) {
		horizontalAngle += mouseSpeed * float(cwidth / 2 - xpos);
		verticalAngle += mouseSpeed * float(cheight / 2 - ypos);
	}

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	glm::vec3 up = glm::cross( right, direction );

	//--------------------------------------------------------------------------- move camera
	if (glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
		position += glm::cross(right, direction) * deltaTime * speed;
	}
	if (glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
		position -= glm::cross(right, direction) * deltaTime * speed;
	}
	if (glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}

	float FoV = initialFoV;

	ProjectionMatrix = glm::perspective(glm::radians(FoV), 16.0f / 9.0f, 0.1f, 150.0f);
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	
	lastTime = currentTime;
}