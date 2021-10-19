#include <GLFW\glfw3.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "OpenGL Example", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	const float DEG2RAD = 3.14159 / 180;
	float radius = 0.2;
	float r = 0.0;
	float g = 0.3;
	float b = 0.6;
	float x = 0.5;
	float y = -0.33;
	bool ballLeft = true;
	bool ballDown = true;
	float speed = 0.003;
	while (!glfwWindowShouldClose(window)) {
		//Setup View
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		//Game step
		
		//Drawing
		glBegin(GL_POINTS);
		for (int i = -width; i < width; i++) {
			for (int j = -height; j < height; j++) {
				glColor3f(fmod((i + width) / double(width), 1), fmod((j + height) / double(height), 1), 0);
				glVertex2f(i / double(width), j / double(height));
			}
		}
		glEnd();
		//Swap buffer and check for events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}