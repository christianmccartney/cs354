#ifndef CONTROLS_HPP
#define CONTROLS_HPP
void lookEnable();
void toggleFreeMouse();
void togglePosition();
void resetCamera();
void isFocus(int i);
void isMousePress(int i);
void initControls(int width, int height, int x, int y, int z);
void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif