#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;
    GLuint id() const { return m_programId; }

    void setMat4(const char* name, const glm::mat4& value) const;
    void setVec3(const char* name, const glm::vec3& value) const;
    void setFloat(const char* name, float value) const;
    void setInt(const char* name, int value) const;

private:
    GLuint m_programId = 0;

    static GLuint compile(GLenum type, const std::string& source);
};
