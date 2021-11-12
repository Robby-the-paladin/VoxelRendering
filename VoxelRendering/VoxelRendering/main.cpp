#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Tree.h"
#include <sys/timeb.h>
#include <math.h>

#include <iostream>

#define M_PI 3.1415926535897932384626433832795

int get_milli_count() {
    timeb tb;
    ftime(&tb);
    int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
    return nCount;
}

// Function for conversion
double degree_to_rad(double degree) {
    return (degree * (M_PI / 180));
}

GLFWwindow* window;
float last_time;
float cam_dir[3];

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
double yaw = 0, pitch = 0;

Tree tree;

void init(Shader* shader) {
    vector<vector<vector<Voxel>>> mat;
    mat.resize(64);
    for (int i = 0; i < 64; i++) {
        mat[i].resize(64);
        for (int j = 0; j < 64; j++) {
            mat[i][j].resize(64);
            for (int k = 0; k < 64; k++) {
                mat[i][j][k].color = Color(200, 150, 0, 255);
                mat[i][j][k].empty = true;
                mat[i][j][k].reflection_k = 0;
            }
        }
    }
    tree.build(mat);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    last_time = get_milli_count();

    cam_dir[0] = 1;
    cam_dir[1] = 1;
    cam_dir[2] = 1;
}

void mouse_callback()
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    GLfloat yoffset = -(xpos - SCR_WIDTH / 2.0);
    GLfloat xoffset = SCR_HEIGHT / 2.0 - ypos;

    GLfloat sensitivity = 0.01;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    cam_dir[0] = cos(degree_to_rad(yaw)) * cos(degree_to_rad(pitch));
    cam_dir[1] = sin(degree_to_rad(pitch));
    cam_dir[2] = sin(degree_to_rad(yaw)) * cos(degree_to_rad(pitch));

    glfwSetCursorPos(window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
}

void data_packing(Shader* shader,
    float cam_res_x, float cam_res_y,
    float cam_pos_x, float cam_pos_y, float cam_pos_z,
    float cam_dir_x, float cam_dir_y, float cam_dir_z,
    float cam_dist, float viewing_angle) {

    tree.shader_serializing(shader, Vec3(max(0, int(cam_pos_x - cam_dist)),
        max(0, int(cam_pos_y - cam_dist)), max(0, int(cam_pos_z - cam_dist))),
        Vec3(min(tree.max_size, int(cam_pos_x + cam_dist + 1)),
            min(tree.max_size, int(cam_pos_y + cam_dist + 1)), min(tree.max_size, int(cam_pos_z + cam_dist + 1))));

    shader->set2f("cam.resolution", cam_res_x, cam_res_y);
    shader->set3f("cam.pos", cam_pos_x, cam_pos_y, cam_pos_z);
    shader->set3f("cam.dir", cam_dir_x, cam_dir_y, cam_dir_z);
    shader->setFloat("cam.render_distance", cam_dist);
    shader->setFloat("cam.viewing_angle", viewing_angle);

}

void step(Shader* shader) {
    tree.set(Vec3(4, 4, 4), Vec3(5, 5, 5), Voxel(Color(255, 0, 0, 1), 1, 0));

    data_packing(shader,                    // shader pointer
        SCR_WIDTH, SCR_HEIGHT,              // camera resolution
        0, 0, 0,                            // camera position
        cam_dir[0], cam_dir[1], cam_dir[2], // camera direction
        20,                                 // render distance
        M_PI / 4.0);                        // viewing angle

    glfwGetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    mouse_callback();
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shader.vert", "shader.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices_left[] = {
        // positions         // colors
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
        -1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f   // top left
    };

    // set vertex data 
    float vertices[] = {
       1.0f,  1.0f, 0.0f, 1.0f, 0.0, 0.0f,  // top right // 0
       1.0f, -1.0f, 0.0f, 0.0f, 1.0, 0.0f, // bottom right // 1
      -1.0f, -1.0f, 0.0f, 0.0f, 0.0, 1.0f,  // bottom left // 2
      -1.0f,  1.0f, 0.0f, 1.0f, 1.0, 1.0f   // top left  // 3
    };

    // index buffer // Element Buffer Objects (EBO)
    unsigned int indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    // set vertex buffer object anb vertex array object and element buffer objects 
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind vertex array object
    glBindVertexArray(VAO);

    // bind vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // bind element buffer objects
    // EBO is stored in the VAO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // registered VBO as the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // unbind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    init(&ourShader);
    // render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // GAME STEP
        step(&ourShader);

        // draw triangle
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers
        glfwSwapBuffers(window);

        // glfw: poll IO events (keys & mouse)
        glfwPollEvents();

    }

    // de-allocate all resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(ourShader.ID);

    // glfw: terminate and clear all previously GLFW allocated resources
    glfwTerminate();

    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}