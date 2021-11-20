//#define DEBUG
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Tree.h"
#ifdef DEBUG
#include "AuxLib.h"
#endif // DEBUG
#include <math.h>
#include <glm.hpp>
#include <queue>

#include <iostream>

#define M_PI 3.1415926535897932384626433832795

// Function for conversion
double degree_to_rad(double degree) {
    return (degree * (M_PI / 180));
}

GLFWwindow* window;
bool keys[1024];
queue<float> fps;
float last_time;
glm::vec3 cam_dir;
glm::vec3 cam_pos;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;
float render_distance = 512;
float camera_speed = 0.2f;
double yaw = 0, pitch = 0;

Tree tree;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (action == GLFW_PRESS)
        keys[key] = true;
    else if (action == GLFW_RELEASE)
        keys[key] = false;
}

void init(Shader* shader) {
    vector<vector<vector<Voxel>>> mat;
    int _size = 64;

    /*mat.resize(_size);
    for (int i = 0; i < _size; i++) {
        mat[i].resize(_size);
        for (int j = 0; j < _size; j++) {
            mat[i][j].resize(_size);
            for (int k = 0; k < _size; k++) {
                mat[i][j][k].color = Color(0, 200, 0, 255);
                mat[i][j][k].empty = true;
                mat[i][j][k].reflection_k = 0;
                if (k * 0.1 < sin(i * 0.1) * 3 + sin(j * 0.23) * 3.5)
                    mat[i][j][k].empty = false;
            }
        }
    }
    tree.build(mat, shader);*/
    tree.load_vox_file("example.ex", shader);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetKeyCallback(window, key_callback);
#ifdef DEBUG
    last_time = aux::get_milli_count();
#endif // DEBUG

    cam_dir[0] = 1;
    cam_dir[1] = 1;
    cam_dir[2] = 1;
}

void mouse_callback()
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    GLfloat xoffset = xpos - SCR_WIDTH / 2.0;
    GLfloat yoffset = SCR_HEIGHT / 2.0 - ypos;


    GLfloat sensitivity = 0.05;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.y = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.z = sin(glm::radians(pitch));
    front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam_dir = glm::normalize(front);

    glfwSetCursorPos(window, SCR_WIDTH / 2, SCR_HEIGHT / 2);
}

void do_movement() {
    GLfloat cameraSpeed = camera_speed;
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 camera_right = glm::normalize(glm::cross(up, cam_dir));
    glm::vec3 camera_up = glm::cross(cam_dir, camera_right);

    glm::vec3 move_dir = glm::vec3(0.0f, 0.0f, 0.0f);
    if (keys[GLFW_KEY_W])
        move_dir += cam_dir;
    if (keys[GLFW_KEY_S])
        move_dir -= cam_dir;
    if (keys[GLFW_KEY_A])
        move_dir -= glm::normalize(glm::cross(cam_dir, camera_up));
    if (keys[GLFW_KEY_D])
        move_dir += glm::normalize(glm::cross(cam_dir, camera_up));
    if (keys[GLFW_KEY_SPACE])
        move_dir += up;
    if (keys[GLFW_KEY_LEFT_CONTROL])
        move_dir -= up;
    if (keys[GLFW_KEY_LEFT_SHIFT]) {
        cameraSpeed = camera_speed * 2;
    } else {
        cameraSpeed = camera_speed;
    }
    if (glm::dot(glm::abs(move_dir), glm::vec3(1, 1, 1)) > 0.0001)
        cam_pos += cameraSpeed * glm::normalize(move_dir);
}

void data_packing(Shader* shader,
    float cam_res_x, float cam_res_y,
    float cam_pos_x, float cam_pos_y, float cam_pos_z,
    float cam_dir_x, float cam_dir_y, float cam_dir_z,
    float cam_dist, float viewing_angle) {

    tree.shader_serializing(shader);

    shader->set2f("cam.resolution", cam_res_x, cam_res_y);
    shader->set3f("cam.pos", cam_pos_x, cam_pos_y, cam_pos_z);
    shader->set3f("cam.dir", cam_dir_x, cam_dir_y, cam_dir_z);
    shader->setFloat("cam.render_distance", cam_dist);
    shader->setFloat("cam.viewing_angle", viewing_angle);

}

void step(Shader* shader) {
    data_packing(shader,                    // shader pointer
        SCR_WIDTH, SCR_HEIGHT,              // camera resolution
        cam_pos.x, cam_pos.y, cam_pos.z,    // camera position
        cam_dir.x, cam_dir.y, cam_dir.z,    // camera direction
        render_distance,                                 // render distance
        M_PI / 2.0);                        // viewing angle

    glfwGetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    mouse_callback();
    do_movement();
#ifdef DEBUG
    fps.push(aux::get_milli_count());
    while (aux::get_milli_count() - fps.front() > 1000) {
        fps.pop();
    }
    cout << "fps: " << fps.size() << " ";
#endif // DEBUG
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
#ifdef DEBUG
    int step_time = aux::get_milli_count();
    int render_time = aux::get_milli_count();
#endif // DEBUGG
    // render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // GAME STEP
        //glFinish();
        //glFlush();
#ifdef DEBUG
        render_time = aux::get_milli_count() - render_time;
        step_time = aux::get_milli_count();
#endif // DEBUG
        step(&ourShader);
#ifdef DEBUG
        step_time = aux::get_milli_count() - step_time;
        cout << "stime: " << step_time << " rtime: " << render_time << endl;
        render_time = aux::get_milli_count();
#endif // DEBUG

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