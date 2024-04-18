#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Noise3D.c"

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

const unsigned int NUM_PARTICLES = 200;

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float size;
    float lifetime;
    float curtime;

    Particle() : position(0.0f), velocity(0.0f), lifetime(0.0f) {}
    Particle(glm::vec3 pos, glm::vec3 vel) : position(pos), velocity(vel) {}
};

std::vector<Particle> particles;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLuint loadTexture(std::filesystem::path filePath) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char* data = stbi_load(filePath.string().c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

void initParticles() {
    particles.resize(5000);
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i].position = glm::vec3(0,0,0);
        particles[i].velocity = glm::vec3(0,0,0);
        particles[i].size = 0.0f;
        particles[i].lifetime = 0.0f;
        particles[i].curtime = 0.0f;
    }
}

void SetupVertexAttributes(GLuint vboID)
{
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    //position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);
    glEnableVertexAttribArray(0);

    //velocity
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    //size
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    //lifetime
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(3);

    //curtime
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);

}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Particle System", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    //shader
    // 在初始化时指定要捕获的varying变量
    const char* feedbackVaryings[] = { "outPos","outVel","outSize","outLifetime","outCurtime"};

    Shader emitShader("emit.vert", "emit.frag", feedbackVaryings, 5);
    Shader drawShader("draw.vert", "draw.frag");

    // 初始化粒子
    initParticles();

    GLuint curSrcIndex = 0;

    std::filesystem::path filePath = "textures/smoke.tga";
    GLuint textureId = loadTexture(filePath);

    GLuint noiseTextureId = Create3DNoiseTexture(128, 50.0);

    unsigned int particleVBO[2];
    glGenBuffers(2, &particleVBO[0]);
    for (int i = 0; i < 2; i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO[i]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * NUM_PARTICLES,particles.data(), GL_DYNAMIC_COPY);
    }

    float uTime = 0.f;
    GLsync emitSync;
    unsigned int myTBOquery;

    // Loop until the user closes the window
            // Bind the texture

    while (!glfwWindowShouldClose(window)) {
        //---------------------------------------------------emit particles--------------------------------------------------------
        uTime += 0.001;
        //读取
        GLuint srcVBO = particleVBO[curSrcIndex];
        //输出
        GLuint dstVBO = particleVBO[(curSrcIndex + 1) % 2];

        emitShader.use();

        SetupVertexAttributes(srcVBO);

        // Set transform feedback buffer,feedback buffer info destination
        glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, dstVBO);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, dstVBO);

        // 启用变换反馈,禁止绘制
        glEnable(GL_RASTERIZER_DISCARD);

        emitShader.setFloat("u_time", uTime);
        emitShader.setFloat("u_emissionRate", 0.3);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D,noiseTextureId);
        emitShader.setInt("s_noiseTex", 0);

        glGenQueries(1, &myTBOquery);
        // 开始变换反馈
        glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, myTBOquery);
        glBeginTransformFeedback(GL_POINTS);

        // 绘制粒子
        glDrawArrays(GL_POINTS, 0, particles.size());

        // 结束变换反馈
        glEndTransformFeedback();
        glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
        unsigned int numEdgesFedBack;
        glGetQueryObjectuiv(myTBOquery, GL_QUERY_RESULT, &numEdgesFedBack);
        std::cout << numEdgesFedBack<<std::endl;

        // 绑定缓冲区对象
        glBindBuffer(GL_ARRAY_BUFFER, dstVBO);

        // 映射缓冲区对象到客户端内存
        void* dataPtr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
        if (dataPtr != nullptr) {
            float* dataArray = static_cast<float*>(dataPtr);
            for (int i = 0; i < 9; ++i) {
                std::cout << "Data[" << i << "] = " << dataArray[i] << std::endl;
            }

            // 取消映射缓冲区对象
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
        else {
            // 映射失败，处理错误
            std::cerr << "Failed to map buffer object" << std::endl;
        }

        // 解绑缓冲区对象
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Create a sync object to ensure transform feedback results are completed before the draw that uses them.
        emitSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // 禁用变换反馈，开启绘制
        glDisable(GL_RASTERIZER_DISCARD);

        //取消绑定
        glUseProgram(0);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //ping pong the buffers
        curSrcIndex = (curSrcIndex + 1) % 2;

        //---------------------------------------------------draw start--------------------------------------------------------
        // Block the GL server until transform feedback results are completed
        glWaitSync(emitSync, 0, GL_TIMEOUT_IGNORED);
        glDeleteSync(emitSync);

        // Set the viewport
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(1, 1, 1, 0);
        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(0x8861);
        
        drawShader.use();   

        SetupVertexAttributes(particleVBO[curSrcIndex]);

        //unifrom set
        drawShader.setFloat("u_time", uTime);
        drawShader.setVec3("u_acceleration", glm::vec3(0,-1,0));
        drawShader.setVec4("u_color",glm::vec4(1.0f));
        drawShader.setInt("s_texture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);

        //Blend particles
        glEnable(GL_BLEND);
        //和背景颜色的alpha混合
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
        //------------------------------------------------ draw end---------------------------------------------------------------------------
        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up
    glDeleteBuffers(2, &particleVBO[0]);
    glfwTerminate();
    return 0;
}
