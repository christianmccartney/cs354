#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <complex>
#include <windows.h> 
#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>

#define SCREENWIDTH 1600
#define SCREENHEIGHT 900
#define FIELDX 100
#define FIELDY 200
#define FIELDZ 200
#define EPSILON 1.0e-5
#define PI 3.14159

struct Particle{
	glm::vec3 pos, speed;
	unsigned char r,g,b,a;					// Color
	float size, angle, weight;
	float life;								// Remaining life of the particle. if <0 : dead and unused.
	float cameradistance;					// *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		return this->cameradistance > that.cameradistance;
	}
};

std::map<int, int> mapx;
std::map<int, int> mapy;
double particlerate = 500.0;
double oldparticlerate = 0.0;
const int MaxParticles = 100000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;
float gravity = 0.0f;
float oldgravity = -9.81f;
int isforce = 0;
float forcemod = 0.1;
float fd = 0.03f;
float particleweight = 10.0f;
float particlesize = 0.75f;

glm::vec3 plane2_1 = glm::vec3(0, 0, (3 / 4)*FIELDZ);
glm::vec3 plane2_2 = glm::vec3(FIELDX, FIELDY, (3 / 4)*FIELDZ);

glm::vec3 vecfield1[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield2[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield3[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield4[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield5[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield6[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield7[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield8[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield9[FIELDX][FIELDY][FIELDZ];
glm::vec3 vecfield0[FIELDX][FIELDY][FIELDZ];

int i, j, k, px, py, pz;
int forceselector = 1;
int isdispersing = 1;
int isall = 0;
int isstreamon = 0;

int isplane1 = 0;
int isplane2 = 0;
glm::vec3 A = glm::vec3(0.0, 0.0, (0.75) * FIELDZ);
glm::vec3 B = glm::vec3(FIELDX, FIELDY, (0.75) * FIELDZ);
glm::vec3 C = glm::vec3(0.0, FIELDY, (0.75)  * FIELDZ);

glm::vec3 D = glm::vec3(0.0, 0.0, (0.25) * FIELDZ);
glm::vec3 E = glm::vec3(FIELDX, FIELDY, (0.25) * FIELDZ);
glm::vec3 F = glm::vec3(0.0, FIELDY, (0.25)  * FIELDZ);

glm::vec3 N = glm::normalize(glm::cross((E - D), (D - F)));
glm::vec3 N2 = glm::normalize(glm::cross((B - A), (A - C)));

float a = 0.0;
float b = 0.0;
float c = 0.0;
float fa = 0.0;
float fc = 0.0;
int isdone = 1;
glm::vec3 vn = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 vt = glm::vec3(0.0, 0.0, 0.0);

int isMouseLocked = 1;
int mousePressed = 0;
int isMouseField = 0;

int isRepulsorOn = 1;
int isSettingRepulsor = 0;
int repulsorFieldSize = 15;
float repulsorFieldStrength = 5;
int isRepulsorSet = 0;

int isAttractorOn = 1;
int isSettingAttractor = 0;
int attractorFieldSize = 15;
float attractorFieldStrength = 5;
int isAttractorSet = 0;

glm::vec3 initialdirection = glm::vec3(0.0f, 0.0f, 10.0f);

int rightMouseToggle = 1;

double xpos, ypos;
int xp, yp;

int isLookEnable = 0;

glm::vec3 repulsorfield[FIELDX][FIELDY][FIELDZ];
glm::vec3 attractorfield[FIELDX][FIELDY][FIELDZ];

int sign(int x) {
	return (x > 0) - (x < 0);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	//--------------------------------------------------------------------------- toggle force fields
	if (key == GLFW_KEY_1 && action == GLFW_RELEASE)
		forceselector = 1;
	if (key == GLFW_KEY_2 && action == GLFW_RELEASE)
		forceselector = 2;
	if (key == GLFW_KEY_3 && action == GLFW_RELEASE)
		forceselector = 3;
	if (key == GLFW_KEY_4 && action == GLFW_RELEASE)
		forceselector = 4;
	if (key == GLFW_KEY_5 && action == GLFW_RELEASE)
		forceselector = 5;
	if (key == GLFW_KEY_6 && action == GLFW_RELEASE)
		forceselector = 6;
	if (key == GLFW_KEY_7 && action == GLFW_RELEASE)
		forceselector = 7;
	if (key == GLFW_KEY_8 && action == GLFW_RELEASE)
		forceselector = 8;
	if (key == GLFW_KEY_9 && action == GLFW_RELEASE)
		forceselector = 9;
	if (key == GLFW_KEY_0 && action == GLFW_RELEASE)
		forceselector = 0;

	//--------------------------------------------------------------------------- toggle mouse cursor
	if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
		if (isMouseLocked) {
			isMouseLocked = 0;
			toggleFreeMouse();
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPos(window, SCREENWIDTH / 2, SCREENHEIGHT / 2);
		} else if (mousePressed || isSettingAttractor || isSettingRepulsor) {
			std::cout << "error mouse locked" << std::endl;
		} else {
			isMouseLocked = 1;
			toggleFreeMouse();
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, SCREENWIDTH / 2, SCREENHEIGHT / 2);
		}
	}

	//--------------------------------------------------------------------------- toggle camera freelook
	if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
		if (mousePressed || isSettingAttractor || isSettingRepulsor || !isMouseLocked) {
			std::cout << "error mouse in use" << std::endl;
		} else if(!isLookEnable){
			isLookEnable = 1;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, SCREENWIDTH / 2, SCREENHEIGHT / 2);
			lookEnable();
		} else {
			isLookEnable = 0;
			lookEnable();
		}
	}
	//--------------------------------------------------------------------------- flip/toggle gravity
	if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
		if(gravity != 0)
			gravity = gravity * -1.0f;
	}
	if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		float temp = gravity;
		gravity = oldgravity;
		oldgravity = temp;
	}

	//--------------------------------------------------------------------------- increase/decrease force field strength
	if (key == GLFW_KEY_T  && action == GLFW_REPEAT) {
		if (forcemod <= 10.0f)
			forcemod = forcemod + 0.1f;
	}
	if (key == GLFW_KEY_G  && action == GLFW_REPEAT) {
		if (forcemod >= 0.1f)
			forcemod = forcemod - 0.1f;
	}

	//--------------------------------------------------------------------------- increase/decrease particle weight
	if (key == GLFW_KEY_RIGHT_SHIFT  && action == GLFW_PRESS) {
		if (particleweight < 640.0f)
			particleweight = particleweight * 2;
	}
	if (key == GLFW_KEY_RIGHT_CONTROL  && action == GLFW_PRESS) {
		if (particleweight > 1.5f)
			particleweight = particleweight / 2;
	}

	//--------------------------------------------------------------------------- toggle attractor/repulsor
	if (key == GLFW_KEY_C  && action == GLFW_RELEASE) {
		if (isRepulsorOn)
			isRepulsorOn = 0;
		else
			isRepulsorOn = 1;
	}
	if (key == GLFW_KEY_X  && action == GLFW_RELEASE) {
		if (isAttractorOn)
			isAttractorOn = 0;
		else
			isAttractorOn = 1;
	}

	if (key == GLFW_KEY_V  && action == GLFW_RELEASE) {
		if (rightMouseToggle)
			rightMouseToggle = 0;
		else
			rightMouseToggle = 1;
	}

	//--------------------------------------------------------------------------- increase/decrease attractor/repulsor size/strength
	if (rightMouseToggle) {
		if (key == GLFW_KEY_COMMA  && action == GLFW_PRESS) {
			attractorFieldSize += 1;
		}
		if (key == GLFW_KEY_PERIOD  && action == GLFW_PRESS) {
			attractorFieldSize -= 1;
		}

		if (key == GLFW_KEY_EQUAL  && action == GLFW_PRESS) {
			attractorFieldStrength += 1;
		}

		if (key == GLFW_KEY_MINUS  && action == GLFW_PRESS) {
			attractorFieldStrength -= 1;
		}
	} else {
		if (key == GLFW_KEY_COMMA  && action == GLFW_PRESS) {
			repulsorFieldSize += 1;
		}
		if (key == GLFW_KEY_PERIOD  && action == GLFW_PRESS) {
			repulsorFieldSize -= 1;
		}

		if (key == GLFW_KEY_EQUAL  && action == GLFW_PRESS) {
			repulsorFieldStrength += 1;
		}

		if (key == GLFW_KEY_MINUS  && action == GLFW_PRESS) {
			repulsorFieldStrength -= 1;
		}
	}

	//--------------------------------------------------------------------------- increase/decrease initial speed
	if (key == GLFW_KEY_W  && action == GLFW_PRESS) {
		if (glm::length(initialdirection) < 40)
			initialdirection = initialdirection * 2.0f;
	}
	if (key == GLFW_KEY_S  && action == GLFW_PRESS) {
		if (glm::length(initialdirection) > 0)
			initialdirection = initialdirection / 2.0f;
	}

	//--------------------------------------------------------------------------- reset
	if (key == GLFW_KEY_Q && action == GLFW_RELEASE) {
		gravity = 0.0f;
		oldgravity = -9.81f;
		isall = 0;
		particleweight = 10.0f;
		initialdirection = glm::vec3(0.0f, 0.0f, 10.0f);
		if(isstreamon)
			particlerate = 500.0f;
		resetCamera();
	}

	//--------------------------------------------------------------------------- toggle force
	if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
		if (isforce)
			isforce = 0;
		else
			isforce = 1;
	}

	//--------------------------------------------------------------------------- increase/decrease particle stream spawn rate
	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_RELEASE) {
		particlerate += 250.0;
	}
	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_RELEASE) {
		if(particlerate>0)
			particlerate -= 250.0;
	}
	if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
		if (isstreamon)
			isstreamon = 0;
		else
			isstreamon = 1;
		double t = particlerate;
		particlerate = oldparticlerate;
		oldparticlerate = t;
	}

	//--------------------------------------------------------------------------- change particle stream direction right/left/up/down/all
	if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		initialdirection = glm::vec3(0.0f, 0.0f, glm::length(initialdirection));
		isall = 0;
	}
	if (key == GLFW_KEY_K && action == GLFW_RELEASE) {
		initialdirection = glm::vec3(0.0f, 0.0f, -glm::length(initialdirection));
		isall = 0;
	}
	if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
		initialdirection = glm::vec3(0.0f, glm::length(initialdirection), 0.0f);
		isall = 0;
	}
	if (key == GLFW_KEY_SEMICOLON && action == GLFW_RELEASE) {
		initialdirection = glm::vec3(0.0f, -glm::length(initialdirection), 0.0f);
		isall = 0;
	}
	if (key == GLFW_KEY_APOSTROPHE && action == GLFW_RELEASE) {
		isall = 1;
	}

	//--------------------------------------------------------------------------- spawn random particles
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		if(isdispersing)
			isdispersing = 0;
		else
			isdispersing = 1;
	}

	//--------------------------------------------------------------------------- toggle planes
	if (key == GLFW_KEY_Y && action == GLFW_RELEASE) {
		if (isplane1)
			isplane1 = 0;
		else
			isplane1 = 1;
	}
	if (key == GLFW_KEY_U && action == GLFW_RELEASE) {
		if (isplane2)
			isplane2 = 0;
		else
			isplane2 = 1;
	}

	//--------------------------------------------------------------------------- move planes left/right
	if (key == GLFW_KEY_H && action == GLFW_RELEASE) {
		A = A + glm::vec3(0.0, 0.0, -10.0f);
		B = B + glm::vec3(0.0, 0.0, -10.0f);
		C = C + glm::vec3(0.0, 0.0, -10.0f);
		glm::vec3 N2 = glm::normalize(glm::cross((B - A), (A - C)));
	}
	if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		A = A + glm::vec3(0.0, 0.0, 10.0f);
		B = B + glm::vec3(0.0, 0.0, 10.0f);
		C = C + glm::vec3(0.0, 0.0, 10.0f);
		glm::vec3 N2 = glm::normalize(glm::cross((B - A), (A - C)));
	}
	if (key == GLFW_KEY_B && action == GLFW_RELEASE) {
		D = D + glm::vec3(0.0, 0.0, -10.0f);
		E = E + glm::vec3(0.0, 0.0, -10.0f);
		F = F + glm::vec3(0.0, 0.0, -10.0f);
		glm::vec3 N = glm::normalize(glm::cross((E - D), (D - F)));
	}
	if (key == GLFW_KEY_N && action == GLFW_RELEASE) {
		D = D + glm::vec3(0.0, 0.0, 10.0f);
		E = E + glm::vec3(0.0, 0.0, 10.0f);
		F = F + glm::vec3(0.0, 0.0, 10.0f);
		glm::vec3 N = glm::normalize(glm::cross((E - D), (D - F)));
	}

	//--------------------------------------------------------------------------- move planes left/right
	if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
		togglePosition();
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (!isMouseLocked) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			isMousePress(1);
			mousePressed = 1;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			isMousePress(0);
			mousePressed = 0;
		}
		if (rightMouseToggle) {
			if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
				mousePressed = 1;
				if (isSettingAttractor) {
					isSettingAttractor = 0;
				}
				else {
					isSettingAttractor = 1;
					isAttractorSet = 0;
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				}
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
				mousePressed = 0;
				if (!isSettingAttractor) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				else {
					isAttractorSet = 1;
				}
			}
		} else {
			if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
				mousePressed = 1;
				if (isSettingRepulsor) {
					isSettingRepulsor = 0;
				}
				else {
					isSettingRepulsor = 1;
					isRepulsorSet = 0;
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
				}
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
				mousePressed = 0;
				if (!isSettingRepulsor) {
					glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				}
				else {
					isRepulsorSet = 1;
				}
			}
		}
	}
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!isMouseLocked && mousePressed) {
		initialdirection = glm::vec3(0.0f, 0.1*((SCREENHEIGHT / 2) - (float)ypos), -0.1*((SCREENWIDTH / 2) - (float)xpos));
	}
}

void windowfocus_callback(GLFWwindow * window, int focused) {
	isFocus(focused);
}


int FindUnusedParticle() {

	for(int i=LastUsedParticle; i<MaxParticles; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	for(int i=0; i<LastUsedParticle; i++){
		if (ParticlesContainer[i].life < 0){
			LastUsedParticle = i;
			return i;
		}
	}

	return 0;
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

int main( void ) {
	
	for (i = 0; i < SCREENWIDTH; i++) {
		mapx.insert(std::make_pair(i, (int)(i / 8)));
	}
	for (i = 0; i < SCREENHEIGHT; i++) {
		mapy.insert(std::make_pair(i, (int)((SCREENHEIGHT - i) / 4.5)));
	}
	
		
	


	initControls(SCREENWIDTH, SCREENHEIGHT, FIELDX, FIELDY, FIELDZ);

	float x = FIELDY/-2;
	float y = FIELDY/-2;

	//--------------------------------------------------------------------------- line with positive slope
	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield1[i][j][k] = glm::vec3(0.0f, x, y);
				x += 1;
			}
			x = FIELDY/-2;
			y += 1;
		}
		y = FIELDY/-2;
	}

	//--------------------------------------------------------------------------- line with negative slope
	x = FIELDY / -2;
	y = FIELDY / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield2[i][j][k] = glm::vec3(0.0f, -x, -y);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
	}


	//--------------------------------------------------------------------------- clockwise circle
	x = FIELDY / -2;
	y = FIELDY / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield3[i][j][k] = glm::vec3(0.0f, -x, y);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
	}

	//--------------------------------------------------------------------------- counter clockwise circle
	x = FIELDY / -2;
	y = FIELDY / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield4[i][j][k] = glm::vec3(0.0f, x, -y);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
	}

	//--------------------------------------------------------------------------- offset clockwise circle
	x = FIELDY / -2;
	y = FIELDY / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield5[i][j][k] = glm::vec3(0.0f, -x + FIELDX/4, y);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
	}

	//--------------------------------------------------------------------------- counter clockwise ellipsoid
	x = FIELDY / -2;
	y = FIELDY / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield6[i][j][k] = glm::vec3(0.0f, x, x-y);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
	}

	//--------------------------------------------------------------------------- sinx, siny
	const std::complex<float> fi(0.0f, 1.0f);
	std::complex<float> xcomp(-1.0f, 0.0f);
	std::complex<float> ycomp(-1.0f, 0.0f);
	std::complex<float> div(2.0f / (float)FIELDY, 0.0f);

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield6[i][j][k] = glm::vec3(0.0f, -1.0*std::imag(sin(xcomp + (fi*ycomp))) * 100, std::real(sin(xcomp + (fi*ycomp))) * 100);
				xcomp = xcomp + div;
			}
			xcomp = std::complex<float>(-1.0f, 0.0f);
			ycomp = ycomp + div;
		}
		ycomp = std::complex<float>(-1.0f, 0.0f);
	}

	//--------------------------------------------------------------------------- imaginary exp = 4
	xcomp = std::complex<float>(0.0f, 0.0f);
	ycomp = std::complex<float>(0.0f, 0.0f);

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield7[i][j][k] = glm::vec3(0.0f, -1.0*std::imag(pow((xcomp + (fi*ycomp)), 4) - 1.0f) * 10, std::real(pow((xcomp + (fi*ycomp)), 4) - 1.0f) * 10);
				xcomp = xcomp + div;
			}
			xcomp = std::complex<float>(0.0f, 0.0f);
			ycomp = ycomp + div;
		}
		ycomp = std::complex<float>(0.0f, 0.0f);
	}
	
	//--------------------------------------------------------------------------- x, y, z

	x = FIELDX / -2;
	y = FIELDY / -2;
	float z = FIELDZ / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield8[i][j][k] = glm::vec3((-y*z)*0.1, (x*z)*0.1, 0.0f);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
		z += 1;
	}

	//--------------------------------------------------------------------------- x, y, z

	x = FIELDX / -2;
	y = FIELDY / -2;
	z = FIELDZ / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield9[i][j][k] = glm::vec3(0.0f, -y*x * 0.1, z*x * 0.1);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
		z += 1;
	}

	//--------------------------------------------------------------------------- x, y, z

	x = FIELDX / -2;
	y = FIELDY / -2;
	z = FIELDZ / -2;

	for (i = 0; i < FIELDX; i++) {
		for (j = 0; j < FIELDY; j++) {
			for (k = 0; k < FIELDZ; k++) {
				vecfield0[i][j][k] = glm::vec3(-y, x, z);
				x += 1;
			}
			x = FIELDY / -2;
			y += 1;
		}
		y = FIELDY / -2;
		z += 1;
	}


	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow( SCREENWIDTH, SCREENHEIGHT, "Particle Field", NULL, NULL);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowFocusCallback(window, windowfocus_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetWindowPos(window, 0, 10);

	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwPollEvents();
	glfwSetCursorPos(window, SCREENWIDTH / 2, SCREENHEIGHT / 2);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	GLuint programID = LoadShaders( "Particle.vertexshader", "Particle.fragmentshader" );

	GLuint CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	
	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];

	for(int i=0; i<MaxParticles; i++){
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}



	GLuint Texture = loadDDS("particle.DDS");

	static const GLfloat g_vertex_buffer_data[] = { 
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};
	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	//--------------------------------------------------------------------------- controls

	SetConsoleTitle("particlefield");

	std::cout << std::endl << std::endl << std::endl << std::endl << std::endl << std::endl;
	std::cout << "|---------------------------------------------------------------------------------------|" << std::endl;
	std::cout << "|\t\t\t\t\tcontrols\t\t\t\t\t|" << std::endl;
	std::cout << "|---------------------------------------------------------------------------------------|" << std::endl;
	std::cout << "| Q to reset camera/gravity\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| W/S to increase/decrease initial particle stream velocity\t\t\t\t|" << std::endl;
	std::cout << "| RIGHT BRACKET/LEFT BRACKET to increase/decrease particle stream spawn rate\t\t|" << std::endl;
	std::cout << "| P to toggle particle stream\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| J/K/L/:/' to shoot particle stream left/right/up/down/all\t\t\t\t|" << std::endl;
	std::cout << "| SPACE to toggle spawn random particles\t\t\t\t\t\t|" << std::endl;
	std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| E to flip gravity\t\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| D to turn off gravity\t\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| Y/U to toggle left/right solid planes\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| H/J and B/N to move right plane left/right and left plane left/right\t\t\t|" << std::endl;
	std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| F to toggle force field\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| T/G to increase/decrease the force field strength\t\t\t\t\t|" << std::endl;
	std::cout << "| 1-0 to select force field\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| M to toggle free mouse\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| LEFT CLICK to shoot particles at mouse when it is toggled on\t\t\t\t|" << std::endl;
	std::cout << "| Z to toggle camera freelook\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| ARROW KEYS to move left/right up/down\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| LEFT SHIFT/LEFT CTRL to move forward/back\t\t\t\t\t\t|" << std::endl;
	std::cout << "| LEFT ALT to toggle between current position and center of field\t\t\t|" << std::endl;
	std::cout << "|\t\t\t\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| RIGHT CLICK to begin placing attractor/repulsor field\t\t\t\t\t|" << std::endl;
	std::cout << "| X to toggle attractor field\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| C to toggle repulsor field\t\t\t\t\t\t\t\t|" << std::endl;
	std::cout << "| V to toggle set repulsor/attractor feld\t\t\t\t\t\t|" << std::endl;
	std::cout << "| ./, to increase/decrease repulsor/attractor field strength (whichever is selected)\t|" << std::endl;
	std::cout << "| =/- to increase/decrease repulsor/attractor field size (whichever is selected)\t|" << std::endl;
	std::cout << "|---------------------------------------------------------------------------------------|" << std::endl;
	

	double lastTime = glfwGetTime();
	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;


		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

		glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		int newparticles = (int)(delta*particlerate);						// in milliseconds

		if (newparticles > (int)(0.016f*10000.0))
			newparticles = (int)(0.016f*10000.0);

		if (isSettingRepulsor) {
			for (i = 0; i < FIELDX; i++) {
				for (j = -repulsorFieldSize - 10; j < repulsorFieldSize + 10; j++) {
					for (k = -repulsorFieldSize - 10; k < repulsorFieldSize + 10; k++) {
						repulsorfield[i][(int)yp + j][(int)xp + k] = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
			}

			glfwGetCursorPos(window, &xpos, &ypos);

			xp = mapx.find((int)xpos)->second;
			yp = mapy.find((int)ypos)->second;

			for (i = 0; i < FIELDX; i++) {
				for (j = -repulsorFieldSize; j < repulsorFieldSize + 1; j++) {
					for (k = -repulsorFieldSize; k < repulsorFieldSize + 1; k++) {
						repulsorfield[i][(int)yp + j][(int)xp + k] = glm::vec3(0.0f, sign(j)*repulsorFieldStrength / sqrt((pow(j, 2) + pow(k, 2))), sign(k)*repulsorFieldStrength / sqrt((pow(j, 2) + pow(k, 2))));
					}
				}
			}
		}

		if (isSettingAttractor) {
			for (i = 0; i < FIELDX; i++) {
				for (j = -attractorFieldSize - 10; j < attractorFieldSize + 10; j++) {
					for (k = -attractorFieldSize - 10; k < attractorFieldSize + 10; k++) {
						attractorfield[i][(int)yp + j][(int)xp + k] = glm::vec3(0.0f, 0.0f, 0.0f);
					}
				}
			}

			glfwGetCursorPos(window, &xpos, &ypos);

			xp = mapx.find((int)xpos)->second;
			yp = mapy.find((int)ypos)->second;

			for (i = 0; i < FIELDX; i++) {
				for (j = -attractorFieldSize; j < attractorFieldSize + 1; j++) {
					for (k = -attractorFieldSize; k < attractorFieldSize + 1; k++) {
						attractorfield[i][(int)yp + j][(int)xp + k] = glm::vec3(0.0f, -1.0*sign(j)*attractorFieldStrength / sqrt((pow(j, 2) + pow(k, 2))), -1.0*sign(k)*attractorFieldStrength / sqrt((pow(j, 2) + pow(k, 2))));
					}
				}
			}
		}
		
		if (isstreamon) {
			for (int i = 0; i < newparticles; i++) {
				int particleIndex = FindUnusedParticle();
				ParticlesContainer[particleIndex].life = 7.0f;
				ParticlesContainer[particleIndex].pos = glm::vec3((float)FIELDX / 2, (float)FIELDY / 2, (float)FIELDZ / 2);

				float spread = 1.5f;
				int select;
				glm::vec3 maindir = glm::vec3(0.0f, 0.0f, 0.0f);
				if (isall) {
					select = rand() % 6;
					switch (select) {
					case 0:
						maindir = glm::vec3(0.0f, 0.0f, 1.0f) * glm::length(initialdirection);
						break;
					case 1:
						maindir = glm::vec3(0.0f, 0.0f, -1.0f) * glm::length(initialdirection);
						break;
					case 2:
						maindir = glm::vec3(0.0f, 1.0f, 0.0f) * glm::length(initialdirection);
						break;
					case 3:
						maindir = glm::vec3(0.0f, -1.0f, 0.0f) * glm::length(initialdirection);
						break;
					case 4:
						maindir = glm::vec3(1.0f, 0.0f, 0.0f) * glm::length(initialdirection);
						break;
					case 5:
						maindir = glm::vec3(-1.0f, 0.0f, 0.0f) * glm::length(initialdirection);
						break;
					}
				} else{
					maindir = initialdirection;
				}

				glm::vec3 randomdir = glm::vec3(
					(rand() % 2000 - 1000.0f) / 1000.0f,
					(rand() % 2000 - 1000.0f) / 1000.0f,
					(rand() % 2000 - 1000.0f) / 1000.0f
				);

				ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;

				ParticlesContainer[particleIndex].r = rand() % 256;
				ParticlesContainer[particleIndex].g = rand() % 256;
				ParticlesContainer[particleIndex].b = rand() % 256;
				ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

				//ParticlesContainer[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;
				ParticlesContainer[particleIndex].size = particlesize;

			}
		}

		if (isdispersing) {
			int randomparticles;
			if (isstreamon) {
				randomparticles = (int)(delta*5000.0);						// 5 every millisecond
			} else {
				randomparticles = (int)(delta*15000.0);
			}
			if (randomparticles > (int)(0.016f*10000.0))
				randomparticles = (int)(0.016f*10000.0);

			for (int i = 0; i < randomparticles; i++) {
				int particleIndex = FindUnusedParticle();
				ParticlesContainer[particleIndex].life = 7.0f;
				ParticlesContainer[particleIndex].pos = glm::vec3( (FIELDX/2) - (float)(rand() % 5) + (float)(rand() % 5 ), (float)(rand() % (FIELDY-1)+1), (float)(rand() % (FIELDZ-1)+1));

				ParticlesContainer[particleIndex].speed = glm::vec3(0.0f, 0.0f, 0.0f);

				ParticlesContainer[particleIndex].r = rand() % 256;
				ParticlesContainer[particleIndex].g = rand() % 256;
				ParticlesContainer[particleIndex].b = rand() % 256;
				ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

				//ParticlesContainer[particleIndex].size = (rand()%1000)/2000.0f + 0.1f;
				ParticlesContainer[particleIndex].size = particlesize;

			}
		}


		int ParticlesCount = 0;
		for (int i = 0; i < MaxParticles; i++) {

			Particle& p = ParticlesContainer[i];
			
				if (p.life > 0.0f) {

					p.life -= delta;
					if (p.life > 0.0f) {
						px = (int)p.pos.x;
						py = (int)p.pos.y;
						pz = (int)p.pos.z;

						if (isRepulsorOn && px > 0 && px < FIELDX && py > 0 && py < FIELDY && pz > 0 && pz < FIELDZ) {
							p.speed += repulsorfield[px][py][pz] / particleweight;
						}
						if (isAttractorOn && px > 0 && px < FIELDX && py > 0 && py < FIELDY && pz > 0 && pz < FIELDZ) {
							p.speed += attractorfield[px][py][pz] / particleweight;
						}

						if (isforce) {
							if (px > 0 && px < FIELDX && py > 0 && py < FIELDY && pz > 0 && pz < FIELDZ) {
								switch (forceselector) {
								case 1:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield1[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 2:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield2[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 3:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield3[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 4:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield4[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 5:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield5[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 6:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield6[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 7:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield7[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 8:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield8[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 9:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield9[px][py][pz] / particleweight - fd * p.speed;
									break;
								case 0:
									p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f + forcemod * vecfield0[px][py][pz] / particleweight - fd * p.speed;
									break;
								}
							}
						}
						else {
							p.speed += glm::vec3(0.0f, gravity, 0.0f) * (float)delta * 0.5f;
						}

						if (isplane2) {
							b = glm::length(p.speed);
							while ((b - a) >= EPSILON && isdone) {
								c = (a + b) / 2.0;
								fa = N2[0] * ((p.pos.x + p.speed.x * a) - A[0]) + N2[1] * ((p.pos.y + p.speed.y * a) - A[1]) + N2[2] * ((p.pos.z + p.speed.z * a) - A[2]);
								fc = N2[0] * ((p.pos.x + p.speed.x * c) - A[0]) + N2[1] * ((p.pos.y + p.speed.y * c) - A[1]) + N2[2] * ((p.pos.z + p.speed.z * c) - A[2]);

								if (fc == 0) {
									isdone = 0;
								}
								else if (fa*fc < 0) {
									b = c;
								}
								else {
									a = c;
								}
							}
							a = 0.0;
							isdone = 1;
							if (c <= 0.1) {
								p.pos.x = ((p.pos.x + p.speed.x * c));
								p.pos.y = ((p.pos.y + p.speed.y * c));
								p.pos.z = ((p.pos.z + p.speed.z * c));
								vn = (glm::dot(N2, p.speed)*N2);
								vt = p.speed - vn;
								p.speed = vt - vn;
							}
						}
						if (isplane1) {
							b = glm::length(p.speed);
							while ((b - a) >= EPSILON && isdone) {
								c = (a + b) / 2.0;
								fa = N[0] * ((p.pos.x + p.speed.x * a) - D[0]) + N[1] * ((p.pos.y + p.speed.y * a) - D[1]) + N[2] * ((p.pos.z + p.speed.z * a) - D[2]);
								fc = N[0] * ((p.pos.x + p.speed.x * c) - D[0]) + N[1] * ((p.pos.y + p.speed.y * c) - D[1]) + N[2] * ((p.pos.z + p.speed.z * c) - D[2]);

								if (fc == 0) {
									isdone = 0;
								}
								else if (fa*fc < 0) {
									b = c;
								}
								else {
									a = c;
								}
							}
							a = 0.0;
							isdone = 1;
							if (c <= 0.1) {
								p.pos.x = ((p.pos.x + p.speed.x * c));
								p.pos.y = ((p.pos.y + p.speed.y * c));
								p.pos.z = ((p.pos.z + p.speed.z * c));
								vn = (glm::dot(N, p.speed)*N);
								vt = p.speed - vn;
								p.speed = vt - vn;
							}
						}
						p.pos += p.speed * (float)delta;

						p.cameradistance = glm::length2(p.pos - CameraPosition);
						g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
						g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
						g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

						g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

						g_particule_color_data[4 * ParticlesCount + 0] = p.r;
						g_particule_color_data[4 * ParticlesCount + 1] = p.g;
						g_particule_color_data[4 * ParticlesCount + 2] = p.b;
						g_particule_color_data[4 * ParticlesCount + 3] = p.a;

					}
					else {
						p.cameradistance = -1.0f;
					}

					ParticlesCount++;

				}
			}
		
		SortParticles();

		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(programID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glUniform1i(TextureID, 0);

		glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		glUniform3f(CameraUp_worldspace_ID   , ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		
		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

		// Draw the particules !
		// This draws many times a small triangle_strip (which looks like a quad).
		// This is equivalent to :
		// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
		// but faster.
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );


	delete[] g_particule_position_size_data;

	// Cleanup VBO and shader
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

