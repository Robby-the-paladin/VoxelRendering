#include "Shader.h"

void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char log[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, log);
            std::cout << "ERROR::COMPILATION_ERROR of type: " << type << "\n" << log << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, log);
            std::cout << "ERROR::LINKING_ERROR: " << type << "\n" << log << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::set3f(const std::string& name, float val1, float val2, float val3) {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), val1, val2, val3);
}

void Shader::set2f(const std::string& name, float val1, float val2) {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), val1, val2);
}


void Shader::use() {
    glUseProgram(ID);
}

void Shader::addTextures(const std::vector<std::string>& names) {
    if (names.size() == 0)
        return;
    // Загрузка изображения с помощью stb_image
    int width, height, nrChannels, depth = names.size();
    stbi_info(names[0].c_str(), &width, &height, &nrChannels);
    int texture_size = width * height * nrChannels;
    void* data = malloc(texture_size * names.size());

    for (int i = 0; i < names.size(); i++) {
        void* pixels = stbi_load(names[i].c_str(), &width, &height, &nrChannels, 0);
        memcpy(((char*)data) + texture_size * i, pixels, texture_size);
        stbi_image_free(pixels);
    }

    // Генерация текстур из картинки
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    if (data)
    {
        glTexImage3D(GL_TEXTURE_2D_ARRAY,
            0,                 // mipmap level
            GL_RGB,            // gpu texel format
            width,             // width
            height,            // height
            depth,             // depth
            0,                 // border
            GL_RGB,            // cpu pixel format
            GL_UNSIGNED_BYTE,  // cpu pixel coord type
            data);           // pixel data
        

        glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

        glTexParameteri(GL_TEXTURE_2D_ARRAY,
            GL_TEXTURE_MIN_FILTER,
            GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,
            GL_TEXTURE_MAG_FILTER,
            GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else
    {
        std::cout << "error: Failed to load texture" << std::endl;
    }


    // Освобождение памяти
    free(data);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vertexShaderFile;
    std::ifstream fragmentShaderFile;
    vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vertexShaderFile.open(vertexPath);
        fragmentShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        vertexShaderFile.close();
        fragmentShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::READ_ERROR" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}