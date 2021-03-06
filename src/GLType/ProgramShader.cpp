/**
 *
 *    \file ProgramShader.cpp
 *
 */


#include <cstdio>
#include <cassert>

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glsw/glsw.h>

#include <tools/gltools.hpp>
#include <tools/Logger.hpp>
#include <GLType/BaseTexture.h>
#include <GLType/ProgramManager.h>
#include <GLType/GraphicsDevice.h>
#include <GLType/OGLGraphicsData.h>
#include <GLType/OGLCoreGraphicsData.h>

#include "ProgramShader.h"

static std::vector<std::string> directory = { ".", "./shaders" };

ProgramShader::ProgramShader() noexcept
    : m_ShaderID(0u)
    , m_BlockPointCounter(0u)
{
}

ProgramShader::~ProgramShader() noexcept
{
    destroy(); 
}

bool ProgramShader::initialize() noexcept
{
    assert(m_ShaderID == GL_NONE);
    if (m_ShaderID != GL_NONE)
        return false;

    m_ShaderID = glCreateProgram();

#ifdef GL_ARB_separate_shader_objects
    glProgramParameteri(m_ShaderID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_FALSE);
    glProgramParameteri(m_ShaderID, GL_PROGRAM_SEPARABLE, GL_FALSE);
#endif
    return true;
}

void ProgramShader::destroy()
{
    if (m_ShaderID) {
        glDeleteProgram(m_ShaderID);
        m_ShaderID = 0;
    }
}

void ProgramShader::addShader(GLenum shaderType, const std::string &tag)
{
    // require initialization
    assert(m_ShaderID > 0);

    if (glswGetError() != 0) {
        fprintf(stderr, "GLSW : %s", glswGetError());
    }
    assert(glswGetError() == 0);

    const char* cTag = tag.c_str();
    const GLchar *source = glswGetShader(cTag);

    if (0 == source)
    {
        fprintf(stderr, "Error : shader \"%s\" not found, check your directory.\n", cTag);
        fprintf(stderr, "Execution terminated.\n");
        exit(EXIT_FAILURE);
    }

    // HACK
    static nv_helpers_gl::IncludeRegistry m_includes;
    static std::vector<std::string> directory = { ".", "./shaders" };
    const std::string content(source);
    const std::string preprocessed = nv_helpers_gl::manualInclude(tag, content, "", directory, m_includes);
    char const* sourcePointer = preprocessed.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &sourcePointer, 0);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        //Logger::getInstance().write( "shader \"%s\" compilation failed.\n", cTag);
        fprintf(stderr, "%s compilation failed.\n", cTag);
        gltools::printShaderLog(shader);
        exit(EXIT_FAILURE);
    }

    //Logger::getInstance().write( "%s compiled.\n", cTag);
    fprintf(stderr, "%s compiled.\n", cTag);

    glAttachShader(m_ShaderID, shader);
    glDeleteShader(shader);     //flag for deletion
}



bool ProgramShader::link()
{
    glLinkProgram(m_ShaderID);

    // Test linking
    GLint status = 0;
    glGetProgramiv(m_ShaderID, GL_LINK_STATUS, &status);

    if(status != GL_TRUE)
    {
        fprintf(stderr, "program linking failed.\n");
        return false;
    }

    return true;
}

bool ProgramShader::initBlockBinding(const std::string& name)
{
    GLint block = glGetUniformBlockIndex(m_ShaderID, name.c_str());
    if (-1 == block)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniformBlockBinding(m_ShaderID, block, m_BlockPointCounter);
    m_BlockPoints.insert({name, m_BlockPointCounter});
    m_BlockPointCounter++;
    return true;
}

void ProgramShader::setDevice(const GraphicsDevicePtr& device)
{
    m_Device = device;
}

bool ProgramShader::setUniform(const std::string &name, GLint v) const
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform1i(loc, v);
    return true;
}


bool ProgramShader::setUniform(const std::string &name, GLfloat v) const
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform1f(loc, v);
    return true;
}

bool ProgramShader::setUniform(const std::string &name, const glm::vec3 &v) const
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform3fv(loc, 1, glm::value_ptr(v));
    return true;
}

bool ProgramShader::setUniform(const std::string &name, const glm::vec4 &v) const
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniform4fv(loc, 1, glm::value_ptr(v));
    return true;
}

bool ProgramShader::setUniform(const std::string &name, const glm::mat3 &v) const
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".\n", name.c_str());
        return false;
    }

    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v));
    return true;
}

bool ProgramShader::setUniform(const std::string &name, const glm::mat4 &v) const
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if(-1 == loc)
    {
        printf("ProgramShader : can't find uniform \"%s\".", name.c_str());
        return false;
    }

    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v));
    return true;
}

bool ProgramShader::bindTexture(const std::string &name, const BaseTexturePtr& texture, GLint unit)
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if (-1 == loc)
    {
        printf("ProgramShader : can't find texture \"%s\".\n", name.c_str());
        return false;
    }

    texture->bind(unit);
    glUniform1i(loc, unit);

    return true;
}

bool ProgramShader::bindBuffer(const std::string& name, const GraphicsDataPtr& data)
{
    auto device = m_Device.lock();
    if (!device) return false;
    auto it = m_BlockPoints.find(name);
    if (it == m_BlockPoints.end())
        return false;

    auto blockPoint = it->second;
    auto type = device->getGraphicsDeviceDesc().getDeviceType();

    // Bind the buffer object to the uniform block
    if (type == GraphicsDeviceType::GraphicsDeviceTypeOpenGLCore)
    {
        auto ubo = data->downcast_pointer<OGLCoreGraphicsData>();
        glBindBufferBase(GL_UNIFORM_BUFFER, blockPoint, ubo->getInstanceID());
        return true;
    }
    else if (type == GraphicsDeviceType::GraphicsDeviceTypeOpenGL)
    {
        auto ubo = data->downcast_pointer<OGLGraphicsData>();
        glBindBufferBase(GL_UNIFORM_BUFFER, blockPoint, ubo->getInstanceID());
        return true;
    }
    return false;
}

bool ProgramShader::bindImage(const std::string &name, const BaseTexturePtr &texture,
    GLint unit, GLint level, GLboolean layered, GLint layer, GLenum access)
{
    GLint loc = glGetUniformLocation(m_ShaderID, name.c_str());

    if (-1 == loc)
    {
        printf("ProgramShader : can't find image \"%s\".\n", name.c_str());
        return false;
    }

    glBindImageTexture(unit, texture->m_TextureID, level, layered, layer, access, texture->m_Format);
    glUniform1i(loc, unit);

    return true;
}

bool ProgramShader::setIncludeFromFile(const std::string &includeName, const std::string &filename)
{
    auto incStr = readTextFile(filename);
    if(incStr.size() == 0)
        return false;
    glNamedStringARB(GL_SHADER_INCLUDE_ARB, includeName.size(), includeName.c_str(), incStr.size(), incStr.data());
    return false;
}

std::vector<char> ProgramShader::readTextFile(const std::string &filename)
{
    if (filename.empty()) 
        return std::vector<char>();

	FILE *fp = 0;
	if (!(fp = fopen(filename.c_str(), "r")))
	{
		printf("Cannot open \"%s\" for read!\n", filename.c_str());
		return std::vector<char>();
	}

    fseek(fp, 0L, SEEK_END);     // seek to end of file
    long size = ftell(fp);       // get file length
    rewind(fp);                  // rewind to start of file

    std::vector<char> buffer(size);

	size_t bytes;
	bytes = fread(buffer.data(), 1, size, fp);

	fclose(fp);
	return buffer;
}