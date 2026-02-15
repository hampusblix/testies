#include "Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>
#include <vector>

GLuint Shader::compile(GLenum type, const std::string& source) {
    const GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(shader, length, nullptr, log.data());
        std::string msg(log.begin(), log.end());
        glDeleteShader(shader);
        throw std::runtime_error("Shader compile failure: " + msg);
    }

    return shader;
}

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    const GLuint vertex = compile(GL_VERTEX_SHADER, vertexSource);
    const GLuint fragment = compile(GL_FRAGMENT_SHADER, fragmentSource);

    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertex);
    glAttachShader(m_programId, fragment);
    glLinkProgram(m_programId);

    GLint success = 0;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!success) {
        GLint length = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(m_programId, length, nullptr, log.data());
        std::string msg(log.begin(), log.end());
        glDeleteProgram(m_programId);
        m_programId = 0;
        throw std::runtime_error("Program link failure: " + msg);
    }
}

Shader::~Shader() {
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
    }
}

void Shader::use() const {
    glUseProgram(m_programId);
}

void Shader::setMat4(const char* name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_programId, name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const char* name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_programId, name), 1, glm::value_ptr(value));
}

void Shader::setFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(m_programId, name), value);
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(m_programId, name), value);
}
