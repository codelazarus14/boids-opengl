#ifndef CONTROLS_HPP
#define CONTROLS_HPP

enum CameraControlType { FIXED, FPS, ORBITAL };

void computeMatricesFromInputs(CameraControlType controlType = FIXED);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif