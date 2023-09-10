#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

#include <GLFW/glfw3.h>

#include <random>
#include <vector>
#include "boids.hpp"

Boid::Boid(glm::vec3 newPos, glm::vec3 newVel, glm::vec3 newColor) {
	pos = newPos;
	vel = newVel;
	color = newColor;
}

std::vector<Boid> Boids;
std::vector<glm::mat4> ModelMatrices;

std::vector<glm::mat4> getModelMatrices() {
	return ModelMatrices;
}

float radius = 4.0f;
float cohesion = 0.25f;
float N = 0.1;
float A = 10;
float r0 = 2.0f;
// gravity pulling boids toward center of viewport
glm::vec3 blackHole = glm::vec3();
float gravity = 0.05f;
float dt = 0.1f;
// change in color between boids with many/few neighbors
float colorChange = 0.25f;

glm::vec3 noise() {
	std::random_device rd;	// a seed source for the random number engine
	std::mt19937 gen(rd());	// mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distrib(-50, 50);

	glm::vec3 random = glm::normalize(glm::vec3(distrib(gen), distrib(gen), distrib(gen)));
	return N * random;
}

glm::vec3 centerPull(Boid b) {
	glm::vec3 distance = b.pos - blackHole;
	return -gravity * glm::normalize(distance);
}

glm::vec3 aimFor(glm::vec3 targetPos, Boid b) {
	return targetPos - b.pos;
}

glm::vec3 threeLaws(int currIdx) {
	int neighborCount = 0;
	glm::vec3 totalPos = glm::vec3();
	glm::vec3 totalVel = glm::vec3();
	glm::vec3 totalRepel = glm::vec3();
	glm::vec3 averageVel = glm::vec3();
	Boid currBoid = Boids[currIdx];

	for (int i = 0; i < Boids.size(); i++) {
		if (i != currIdx) {
			Boid b = Boids[i];
			glm::vec3 distance = currBoid.pos - b.pos;
			if (glm::length(distance) < radius) {
				// if in range, compute forces
				totalPos += b.pos;
				totalVel += b.vel;
				neighborCount++;
				// repulsion depends on distance
				glm::vec3 diff = 1 / r0 * distance;
				float diffMag2 = glm::length2(diff);
				totalRepel += A * glm::normalize(diff) * std::exp(-diffMag2);
			}
		}
	}
	if (neighborCount > 0) {
		// average velocities
		averageVel += glm::vec3(totalVel.x / neighborCount, totalVel.y / neighborCount, totalVel.z / neighborCount);
		glm::vec3 averagePos = glm::vec3(totalPos.x / neighborCount, totalPos.y / neighborCount, totalPos.z / neighborCount);
		// combine average with cohesive/repulsive forces
		averageVel += cohesion * aimFor(averagePos, currBoid);
		averageVel += glm::vec3(totalRepel.x / neighborCount, totalRepel.y / neighborCount, totalRepel.z / neighborCount);
		// change boid colors based on neighbors
		currBoid.color = glm::vec3(0, 1, glm::normalize(colorChange * neighborCount));
	}
	return averageVel;
}

glm::mat4 rotateBetweenVectors(vec3 srcVec, vec3 destVec) {
	srcVec = glm::normalize(srcVec);
	destVec = glm::normalize(destVec);

	float cosTheta = dot(srcVec, destVec);
	glm::vec3 rotationAxis;

	if (cosTheta < -1 + 0.001f) {
		// if vectors are in opposite directions, no "ideal" rotation
		// try z axis first?
		rotationAxis = glm::cross(vec3(0, 0, 1.0f), srcVec);
		if (glm::length2(rotationAxis) < 0.01) {
			// pick x if we fail
			rotationAxis = cross(vec3(1.0f, 0, 0), srcVec);
		}
		rotationAxis = glm::normalize(rotationAxis);

		glm::quat rotation = glm::angleAxis(glm::radians(180.0f), rotationAxis);
		return glm::toMat4(rotation);
	}
	else {
		// axis is just A X B
		rotationAxis = cross(srcVec, destVec);

		float s = std::sqrt((1 + cosTheta) * 2);
		float inverse = 1 / s;

		glm::quat rotation = glm::quat(
			s * 0.5f,
			rotationAxis.x * inverse,
			rotationAxis.y * inverse,
			rotationAxis.z * inverse
		);
		return glm::toMat4(rotation);
	}
}

void createBoids(int nBoids) {
	if (Boids.size() > 0) { Boids.clear(); ModelMatrices.clear(); }

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(-100, 100);

	for (int i = 0; i < nBoids; i++) {
		// place boids at random positions around center w random velocities
		Boid newBoid(
			glm::vec3(distrib(gen), distrib(gen), distrib(gen)),
			glm::normalize(glm::vec3(distrib(gen), distrib(gen), distrib(gen))),
			glm::vec3(0, 1, 0)
		);
		Boids.push_back(newBoid);
		glm::mat4 translateMatrix = glm::translate(glm::mat4(), newBoid.pos);
		ModelMatrices.push_back(translateMatrix);
	}
}

void computeBoidModelMatrices() {

	for (int i = 0; i < Boids.size(); i++) {
		Boid& b = Boids[i];
		b.pos += b.vel * dt;

		glm::vec3 components[3] = {
			threeLaws(i) * dt,
			noise() * std::sqrt(dt),
			centerPull(b) * dt
		};

		// unit length vel
		b.vel = glm::normalize(b.vel + components[0] + components[1] + components[2]);

		glm::mat4 translateMatrix = glm::translate(glm::mat4(), b.pos);
		// rotate arrows to face the direction they're heading in
		glm::mat4 rotationMatrix = rotateBetweenVectors(vec3(0, 1.0f, 0), b.vel);
		ModelMatrices[i] = translateMatrix * rotationMatrix;
	}
}