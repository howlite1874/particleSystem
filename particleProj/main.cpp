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

// Particle VBOs
GLuint particleVBOs[2];

double getCurrentTime() {
	return glfwGetTime();
}


struct Particle {
	glm::vec3 Pos;
	glm::vec3 Vel;
};

std::vector<Particle> generateParticleData() {
	std::vector<Particle> particles;
	particles.resize(numParticles);

	// position
	for (int i = 0; i < numParticles; ++i) {
		float x = ((float)(rand() % 10000) / 4000.0f) - 1.f;
		float y = ((float)(rand() % 10000) / 4000.0f) - 1.f;
		float z = ((float)(rand() % 10000) / 4000.0f) - 1.f;
		glm::vec3 position = glm::vec3(x, y, z);
		particles[i].Pos = position;
	}

	// velocity
	for (int i = 0; i < numParticles; ++i) {
		float x = ((float)(rand() % 10000) / 40000.0f);
		float y = ((float)(rand() % 10000) / 40000.0f);
		float z = ((float)(rand() % 10000) / 40000.0f);
		glm::vec3 velocity = glm::vec3(x, y, z);
		particles[i].Vel = velocity;
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
	const char* feedbackVaryings[1] =
	{
	   "vPos",
	};
	Shader particleShader("particle.vert", "particle.frag", feedbackVaryings,1);

	// generate particle data
	std::vector<Particle> particles = generateParticleData();
	unsigned int particleVAO;
	glGenBuffers(2, &particleVBOs[0]);

	for (int i = 0; i < 2; i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, particleVBOs[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * numParticles, particles.data(), GL_DYNAMIC_COPY);
	}

	//initialize time
	float curTime = 1.0f;

	// render loop
	int loop = 0;
	double sumTime = 0;
	GLuint curSrcIndex = 0;
	while (!glfwWindowShouldClose(window) && loop<10000)
	{
		loop++;
		double startTime = getCurrentTime();
		curTime += 0.0001;
		// input
		processInput(window);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render point sprites
		particleShader.use();
		GLuint srcVBO = particleVBOs[curSrcIndex];
		GLuint dstVBO = particleVBOs[(curSrcIndex + 1) % 2];
		glGenVertexArrays(1, &particleVAO);

		glBindVertexArray(particleVAO);
		glBindBuffer(GL_ARRAY_BUFFER, srcVBO);
		glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_STATIC_DRAW);

		// set attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Pos));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, Vel));
		glEnableVertexAttribArray(1);

		// unbind 
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if (curTime >= 1.0f)
		{
			float centerPos[3];
			float color[4];

			curTime = 0.0f;

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
		glBeginTransformFeedback(GL_POINTS);
		glPointSize(2.0f);
		glDrawArrays(GL_POINTS, 0, numParticles);
		glEndTransformFeedback();
		double endTime = getCurrentTime();
		// compute render time
		double renderTime = endTime - startTime;
		sumTime += renderTime;
		curSrcIndex = (curSrcIndex + 1) % 2;

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	double averageTime = sumTime / loop;
	std::cout << "Average Render time: " << averageTime << std::endl;


	// optional: de-allocate all resources once they've outlived their purpose:
	glDeleteVertexArrays(1, &particleVAO);
	glDeleteBuffers(2, particleVBOs);

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