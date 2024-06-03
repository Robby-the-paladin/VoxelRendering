#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Tree.h"
#include "VOXLoader.h"
#include "AuxLib.h"
#include <math.h>
#include <glm.hpp>
#include <queue>
#include <vector>
#include <random>
#include <fstream>
#include <yaml-cpp/yaml.h>

#include <iostream>

#define M_PI 3.1415926535897932384626433832795

using namespace std;
class Test
{
private:
	GLFWwindow* window;
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 600;
	float render_distance = 10000;

	std::vector<glm::vec3> sc_offsets;
	vector<glm::vec4> borders;
	vector<glm::vec4> grid_buffer;
	vector<int> offsets;
	vector<Sh_node> scene_buffer;
	vector<Tree> trees;

	std::vector<std::string> scenes = { "room.vox", "sphere.vox"};

	void data_packing(Shader* shader,
		float cam_res_x, float cam_res_y,
		float cam_pos_x, float cam_pos_y, float cam_pos_z,
		float cam_dir_x, float cam_dir_y, float cam_dir_z,
		float cam_dist, float viewing_angle, float quantization_distanse, int quantization_depth) {

		shader->setInt("scenes_number", trees.size());
		shader->set2f("cam.resolution", cam_res_x, cam_res_y);
		shader->set3f("cam.pos", cam_pos_x, cam_pos_y, cam_pos_z);
		shader->set3f("cam.dir", cam_dir_x, cam_dir_y, cam_dir_z);
		shader->setFloat("cam.render_distance", cam_dist);
		shader->setFloat("cam.viewing_angle", viewing_angle);
		shader->setFloat("quantization_distanse", quantization_distanse);
		shader->setInt("quantization_depth", quantization_depth);
	}

	void load_scenes() {
		for (auto tree : trees) {
			//cout << "Root color " << tree.root.voxel.color.r << " " << tree.root.voxel.color.g << " " << tree.root.voxel.color.b << "\n";
			offsets.push_back(scene_buffer.size());
			// Finding subroot (minimal root containing beg & end)
			Node* subroot = &tree.root;
			// Init bfs queue
			queue<pair<Node*, pair<int, int>>> q;

			// Serializing
			q.push(make_pair(subroot, make_pair(-1, -1)));
			int k = 0;
			while (!q.empty()) {
				//std::cout << "here " << eqw++ << "\n";
				Node* cur = q.front().first;
				Sh_node node;
				node.terminal_empty_texture_using[0] = cur->terminal;
				if (cur->terminal) {
					node.terminal_empty_texture_using[1] = cur->voxel.empty;
					node.terminal_empty_texture_using[2] = cur->voxel.texture_num;
					node.terminal_empty_texture_using[3] = cur->grid_offset;
				}
				else {
					for (int i = 0; i < 8; i++) {
						if (cur->children[i] != nullptr) {
							q.push(make_pair(cur->children[i], make_pair(k, i)));
						}
					}
				}
				node.color_refl[0] = 1.0 * cur->voxel.color.r / 255.0;
				node.color_refl[1] = 1.0 * cur->voxel.color.g / 255.0;
				node.color_refl[2] = 1.0 * cur->voxel.color.b / 255.0;
				node.color_refl[3] = cur->voxel.reflection_k;
				if (q.front().second.first != -1) {
					scene_buffer[q.front().second.first + offsets.back()].children[q.front().second.second] = k;
				}
				q.pop();
				k++;
				scene_buffer.push_back(node);
			}
		}


		GLuint ssbo;

		Sh_node* s = scene_buffer.data();
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Sh_node) * scene_buffer.size(), s, GL_DYNAMIC_COPY);

		GLuint binding_point_index = 3;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, ssbo);
	}

	void load_buffers() {
		scene_buffer.clear();

		load_scenes();

		GLuint ssbo;
		GLuint binding_point_index;

		int* sh_offsets = offsets.data();

		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * offsets.size(), sh_offsets, GL_DYNAMIC_COPY);

		binding_point_index = 6;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, ssbo);

		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * borders.size(), borders.data(), GL_DYNAMIC_COPY);
		binding_point_index = 10;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, ssbo);

		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * grid_buffer.size(), grid_buffer.data(), GL_DYNAMIC_COPY);
		binding_point_index = 12;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, ssbo);
	}

public:

	void gen_test(int max_size,  int test_size = 500, int seed = 2) {
		glfwInit();
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VoxelRendering", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return;
		}

		Shader shader("shader.vert", "shader.frag");

		// set vertex data 
		float vertices[] = {
		   1.0f,  1.0f, 0.0f,  // top right // 0
		   1.0f, -1.0f, 0.0f,  // bottom right // 1
		  -1.0f, -1.0f, 0.0f,  // bottom left // 2
		  -1.0f,  1.0f, 0.0f   // top left  // 3
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
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// unbind the VAO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // установка метода наложения текстуры GL_REPEAT (стандартный метод наложения)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// Установка параметров фильтрации текстуры
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		std::srand(seed);


		vector<glm::vec3> cam_pos;
		vector<glm::vec3> cam_dir;

		for (int i = 0; i < test_size; i++) {
			cam_pos.push_back(glm::vec3(rand() % max_size, rand() % max_size, rand() % max_size));
		}

		for (int i = 0; i < test_size; i++) {
			cam_dir.push_back(glm::vec3((rand() % 100) * ((rand() % 2) - 1), (rand() % 100) * ((rand() % 2) - 1), (rand() % 100) * ((rand() % 2) - 1)));
			cam_dir.back() = glm::normalize(cam_dir.back());

		}

		vector<int> stime, rtime;
		int render_time = aux::get_milli_count();
		int step_time;

		ofstream fout("output.txt");

		for (int grid_depth = -1; grid_depth < 7; grid_depth++) {
			for (int dynamic = 0; dynamic < 2; dynamic++) {
				for (int cache = 0; cache < 2; cache++) {
					for (int quantization_distanse = 50; quantization_distanse <= 100; quantization_distanse+= 50) {
						for (int quantization_depth = 1; quantization_depth <= 8; quantization_depth++) {
							sc_offsets = {glm::vec3(0, 0, 0), glm::vec3(0, 0, 0) };
							borders.clear();
							grid_buffer.clear();
							offsets.clear();
							scene_buffer.clear();
							trees.clear();
							rtime.clear();
							stime.clear();


							if (cache) {
								for (auto file : scenes) {

									trees.push_back({});

									vector<vector<vector<Voxel>>> mat;
									VOXLoader::load_vox_file(file, mat);
									trees.back().build(mat, grid_buffer, grid_depth);

									borders.push_back(glm::vec4(0, 0, 0, 0));
									borders.push_back(glm::vec4(trees.back().max_size, trees.back().max_size, trees.back().max_size, 0));
								}
							}
							else {
								trees.push_back({});

								vector<vector<vector<Voxel>>> mat;
								VOXLoader::load_vox_files(scenes, sc_offsets, mat);
								trees.back().build(mat, grid_buffer, grid_depth);

								borders.push_back(glm::vec4(0, 0, 0, 0));
								borders.push_back(glm::vec4(trees.back().max_size, trees.back().max_size, trees.back().max_size, 0));
							}

							load_buffers();

							for (int i = 0; i < test_size; i++) {
								if (dynamic) {
									if (cache) {
										if (borders.size() >= 4) {
											borders[2] = borders[2] + glm::vec4(1, 0, 0, 0);
											borders[3] = borders[3] + glm::vec4(1, 0, 0, 0);
										}
									}
									else {
										if (scenes.size() > 1) {
											sc_offsets[1] += glm::vec3(1, 0, 0);
											trees.pop_back();
											trees.push_back(Tree());
											vector<vector<vector<Voxel>>> mat;
											VOXLoader::load_vox_files(scenes, sc_offsets, mat);
											trees.back().build(mat, grid_buffer, grid_depth);

											borders[0] = (glm::vec4(0, 0, 0, 0));
											borders[1] = (glm::vec4(trees.back().max_size, trees.back().max_size, trees.back().max_size, 0));

											load_buffers();
										}
									}
								}


								render_time = aux::get_milli_count() - render_time;
								if (i != 0)
									rtime.push_back(render_time);
								step_time = aux::get_milli_count();

								data_packing(&shader,
									SCR_WIDTH, SCR_HEIGHT,
									cam_pos[i].x, cam_pos[i].y, cam_pos[i].z,
									cam_dir[i].x, cam_dir[i].y, cam_dir[i].z,
									render_distance,                    
									M_PI / 2.0, quantization_distanse, quantization_depth);

								step_time = aux::get_milli_count() - step_time;
								stime.push_back(step_time);
								rtime.push_back(render_time);

								render_time = aux::get_milli_count();


								glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
								glClear(GL_COLOR_BUFFER_BIT);

								// draw triangles
								shader.use();
								glBindVertexArray(VAO);
								glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

								glfwSwapBuffers(window);
								glfwPollEvents();

							}
							render_time = aux::get_milli_count() - render_time;
							rtime.push_back(render_time);
							for (int i = 0; i < test_size; i++) {
									fout << "grid_depth " << grid_depth << " dynamic " << dynamic << " cache " << cache << " quantization_distanse " << quantization_distanse << " quantization_depth " << quantization_depth <<
									" #" << i << " stime " << stime[i] << " rtime " << rtime[i] << "\n";
							}

						}
					}
				}
			}
		}
	}
};

