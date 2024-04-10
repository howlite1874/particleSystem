#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <vector>

#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const int numParticles = 1000;
const int particleSize = 7;


struct Particle {
	float lifeTime;
	glm::vec3 startPos;
	glm::vec3 endPos;
};

std::vector<Particle> generateParticleData() {
	std::vector<Particle> particles;
	particles.resize(numParticles);

	// start position
	for (int i = 0; i < numParticles; ++i) {
		// range [-1,1]
		float x = ((float)(rand() % 10000) / 40000.0f) - 0.125f;
		float y = ((float)(rand() % 10000) / 40000.0f) - 0.125f;
		float z = ((float)(rand() % 10000) / 40000.0f) - 0.125f;
		glm::vec3 position = glm::vec3(x, y, z);
		particles[i].startPos = position;
	}

	// end position
	for (int i = 0; i < numParticles; ++i) {
		// range [-1,1]
		float x = ((float)(rand() % 10000) / 5000.0f) - 1.0f;
		float y = ((float)(rand() % 10000) / 5000.0f) - 1.0f;
		float z = ((float)(rand() % 10000) / 5000.0f) - 1.0f;
		glm::vec3 position = glm::vec3(x, y, z);
		particles[i].endPos = position;
	}

	// Lifetime of particle
	for (int i = 0; i < numParticles; ++i) {
		particles[i].lifeTime = ((float)(rand() % 10000) / 10000.0f);
	}

	return particles;
}

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Partciel System", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// build and compile our shader program
	Shader particleShader("particle.vert", "particle.frag");

	//-----------------------------------------------------------
	// 生成粒子数据
	std::vector<Particle> particles = generateParticleData();

	unsigned int particleVBO, particleVAO;
	glGenVertexArrays(1, &particleVAO);
	glGenBuffers(1, &particleVBO);

	glBindVertexArray(particleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
	glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_STATIC_DRAW);

	// 设置顶点属性指针
	// 生命周期属性
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, lifeTime));
	glEnableVertexAttribArray(0);
	// 开始位置属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, startPos));
	glEnableVertexAttribArray(1);
	// 结束位置属性
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, endPos));
	glEnableVertexAttribArray(2);

	// 取消绑定 VBO 和 VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//initialize time
	float curTime = 1.0f;

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		curTime += 0.0001;
		// input
		processInput(window);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render point sprites
		particleShader.use();
		if (curTime >= 1.0f)
		{
			float centerPos[3];
			float color[4];

			curTime = 0.0f;

			// Pick a new start location and color
			centerPos[0] = ((float)(rand() % 10000) / 10000.0f) - 0.5f;
			centerPos[1] = ((float)(rand() % 10000) / 10000.0f) - 0.5f;
			centerPos[2] = ((float)(rand() % 10000) / 10000.0f) - 0.5f;

			particleShader.setVec3("u_centerPosition", centerPos[0], centerPos[1], centerPos[2]);

			// Random color
			color[0] = ((float)(rand() % 10000) / 20000.0f) + 0.5f;
			color[1] = ((float)(rand() % 10000) / 20000.0f) + 0.5f;
			color[2] = ((float)(rand() % 10000) / 20000.0f) + 0.5f;
			color[3] = 0.5;

			particleShader.setVec4("u_color", color[0], color[1], color[2], color[3]);
		}

		// Load uniform time variable
		particleShader.setFloat("u_time",curTime);

		glBindVertexArray(particleVAO);
		glPointSize(4.0f);
		glDrawArrays(GL_POINTS, 0, numParticles);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &particleVAO);
	glDeleteBuffers(1, &particleVBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}