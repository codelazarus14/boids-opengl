class Boid {
public:
	glm::vec3 pos;
	glm::vec3 vel;
	glm::vec3 color;
	Boid(glm::vec3 newPos, glm::vec3 newVel, glm::vec3 newColor);
};
void createBoids(int nBoids);
std::vector<glm::mat4> getModelMatrices();
std::vector<glm::vec3> getBoidColors();
void computeBoidModelMatrices();