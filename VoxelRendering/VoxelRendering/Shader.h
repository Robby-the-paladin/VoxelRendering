#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "stb_image.h"

class Shader
{
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void set3f(const std::string& name, float val1, float val2, float val3);
    void set2f(const std::string& name, float val1, float val2);
    void addTextures(const std::vector<std::string>& names);

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};