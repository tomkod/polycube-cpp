// OpenGL helper library
// Copyright (C) 2019 Tomasz Dobrowolski
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "glHelpers.h"

enum ShaderAttribLocation
{
    PositionAttribLocation = 0,
    TexCoordAttribLocation,
    NormalAttribLocation,
    ColorAttribLocation
};

bool validateGL()
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        fprintf(stderr, "GL error code %u\n", error);
        return false;
    }
    return true;
}

// Compile a shader object of the specified type using the specified text.
GLuint compileShader(GLenum type, const char* text)
{
    GLuint shader;
    GLint shader_ok;
    GLsizei log_length;
    char info_log[8192];

    shader = glCreateShader(type);
    if (shader != 0)
    {
        glShaderSource(shader, 1, (const GLchar**)&text, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
        if (shader_ok != GL_TRUE)
        {
            fprintf(stderr, "ERROR: Failed to compile %s shader\n",
                (type == GL_COMPUTE_SHADER) ? "compute" :
                ((type == GL_FRAGMENT_SHADER) ? "fragment" : "vertex"));
            glGetShaderInfoLog(shader, sizeof(info_log), &log_length, info_log);
            fprintf(stderr, "ERROR: \n%s\n\n", info_log);
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

#if GLComputeSupported
GLuint compileComputeProgram(const char* cs_text)
{
    if (!cs_text)
    {
        return 0u;
    }

    GLuint program = 0u;
    GLint program_ok;
    GLuint compute_shader = 0u;
    GLsizei log_length;
    char info_log[8192];

    compute_shader = compileShader(GL_COMPUTE_SHADER, cs_text);
    if (compute_shader != 0u)
    {
        // Make the program that connect the two shader and link it.
        program = glCreateProgram();
        if (program != 0u)
        {
            // Attach both shader and link.
            glAttachShader(program, compute_shader);
            glLinkProgram(program);
            glGetProgramiv(program, GL_LINK_STATUS, &program_ok);

            if (program_ok != GL_TRUE)
            {
                fprintf(stderr, "ERROR, failed to link shader program\n");
                glGetProgramInfoLog(program, sizeof(info_log), &log_length, info_log);
                fprintf(stderr, "ERROR: \n%s\n\n", info_log);
                glDeleteProgram(program);
                glDeleteShader(compute_shader);
                program = 0u;
            }
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to load compute shader\n");
    }
    return program;
}
#endif


// Compile a program object using the specified vertex and fragment text.
GLuint compileShaderProgram(const char* vs_text, const char* fs_text)
{
    if (!vs_text || !fs_text)
    {
        return 0u;
    }

    GLuint program = 0u;
    GLint program_ok;
    GLuint vertex_shader = 0u;
    GLuint fragment_shader = 0u;
    GLsizei log_length;
    char info_log[8192];

    vertex_shader = compileShader(GL_VERTEX_SHADER, vs_text);
    if (vertex_shader != 0u)
    {
        fragment_shader = compileShader(GL_FRAGMENT_SHADER, fs_text);
        if (fragment_shader != 0u)
        {
            // Make the program that connect the two shader and link it.
            program = glCreateProgram();
            if (program != 0u)
            {
                // Attach both shader and link.
                glAttachShader(program, vertex_shader);
                glAttachShader(program, fragment_shader);
                glBindAttribLocation(program, PositionAttribLocation, "aVertex");
                glBindAttribLocation(program, TexCoordAttribLocation, "aTexCoord");
                glBindAttribLocation(program, NormalAttribLocation, "aNormal");
                glBindAttribLocation(program, ColorAttribLocation, "aColor");
                glLinkProgram(program);
                glGetProgramiv(program, GL_LINK_STATUS, &program_ok);

                if (program_ok != GL_TRUE)
                {
                    fprintf(stderr, "ERROR, failed to link shader program\n");
                    glGetProgramInfoLog(program, sizeof(info_log), &log_length, info_log);
                    fprintf(stderr, "ERROR: \n%s\n\n", info_log);
                    glDeleteProgram(program);
                    glDeleteShader(fragment_shader);
                    glDeleteShader(vertex_shader);
                    program = 0u;
                }
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unable to load fragment shader\n");
            glDeleteShader(vertex_shader);
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to load vertex shader\n");
    }
    return program;
}

void setUniformf(GLuint uloc, const glsl_math::vec2& v)
{
    glUniform2f(uloc, static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y));
}
void setUniformf(GLuint uloc, const glsl_math::vec3& v)
{
    glUniform3f(uloc, static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y), static_cast<GLfloat>(v.z));
}
void setUniformf(GLuint uloc, const glsl_math::vec4& v)
{
    glUniform4f(uloc, static_cast<GLfloat>(v.x), static_cast<GLfloat>(v.y),
        static_cast<GLfloat>(v.z), static_cast<GLfloat>(v.w));
}
void setUniformf(GLuint uloc, const glsl_math::mat2& m)
{
    GLfloat mf[4];
    convert(m, mf);
    glUniformMatrix2fv(uloc, 1, GL_FALSE, mf);
}
void setUniformf(GLuint uloc, const glsl_math::mat3& m)
{
    GLfloat mf[9];
    convert(m, mf);
    glUniformMatrix3fv(uloc, 1, GL_FALSE, mf);
}
void setUniformf(GLuint uloc, const glsl_math::mat4& m)
{
    GLfloat mf[16];
    convert(m, mf);
    glUniformMatrix4fv(uloc, 1, GL_FALSE, mf);
}

void setProjectionMatrix(GLuint program, const glsl_math::mat4& m)
{
    GLuint uloc = glGetUniformLocation(program, "uProjectionMatrix");
    setUniformf(uloc, m);
}

void setModelViewMatrix(GLuint program, const glsl_math::mat4& m, bool normal_matrix)
{
    using namespace glsl_math;
    GLuint uloc = glGetUniformLocation(program, "uModelViewMatrix");
    setUniformf(uloc, m);
    if (normal_matrix) {
        uloc = glGetUniformLocation(program, "uNormalMatrix");
        setUniformf(uloc, transpose(inverse(mat3(m))));
    }
}

constexpr static GLuint GLMeshStride[6] = {
    0,
    12, // XYZ
    12 + 8, // XYZUV
    12 + 8 + 12, // XYZUVN
    12 + 8 + 12 + 16, // XYZUVNC
    16 + 16 + 12 + 16 // PTNC
};

GLMesh::GLMesh(Format format, GLuint vertexCount, GLuint indexCount, Primitive primitive)
    : format(format)
    , primitive(primitive)
    , vertexCount(vertexCount)
    , indexCount(indexCount)
    , isBound(false)
{
    glGenVertexArrays(1, &arrayBuffer);

    GLuint buffers[2];
    glGenBuffers(2, buffers);
    vertexBuffer = buffers[0];
    indexBuffer = buffers[1];
}

GLMesh::GLMesh()
    : GLMesh(None, 0, 0, Primitive::TRIANGLES)
{
}

GLMesh::GLMesh(Format format, const GLfloat* vertexData, GLuint vertexCount,
    const GLuint* indexData, GLuint indexCount, Primitive primitive)
    : GLMesh(format, vertexCount, indexCount, primitive)
{
    bind();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexCount, indexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount, vertexData, GL_STATIC_DRAW);

    updateVertexAttributes();

    unbind();
}

#if GLComputeSupported
void GLMesh::initComputeVertices(Format newFormat, GLuint newVertexCount)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);

    newVertexCount *= GLMeshStride[newFormat] >> 2;

    if (newVertexCount > vertexCount)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLfloat) * newVertexCount, 0, GL_DYNAMIC_COPY);
    }
    else
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLfloat) * newVertexCount, 0);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    if (newVertexCount > vertexCount || format != newFormat)
    {
        format = newFormat;

        const bool autoUnbind = bind();
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        updateVertexAttributes();

        if (autoUnbind)
            unbind();
    }

    vertexCount = newVertexCount;
}

void GLMesh::initComputeIndices(GLuint newIndexCount)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, indexBuffer);

    if (newIndexCount > indexCount)
    {
        // Initialize new GL buffer.
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * newIndexCount, 0, GL_DYNAMIC_COPY);
    }
    else
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * newIndexCount, 0);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    const bool autoUnbind = bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    if (autoUnbind)
        unbind();

    indexCount = newIndexCount;
}
#endif

void GLMesh::updateVertices(Format newFormat, const GLfloat* vertexData, GLuint newVertexCount)
{
    const bool autoUnbind = bind();

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    if (newVertexCount > vertexCount)
    {
        // Initialize new GL buffer.
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * newVertexCount, vertexData, GL_DYNAMIC_DRAW);
        format = newFormat;
        updateVertexAttributes();
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * newVertexCount, vertexData);
        if (format != newFormat) {
            format = newFormat;
            updateVertexAttributes();
        }
    }

    if (autoUnbind)
        unbind();

    vertexCount = newVertexCount;
}

void GLMesh::updateIndices(const GLuint* indexData, GLuint newIndexCount, Primitive newPrimitive)
{
    const bool autoUnbind = bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    if (newIndexCount > indexCount)
    {
        // Initialize new GL buffer.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * newIndexCount, indexData, GL_DYNAMIC_DRAW);
    }
    else
    {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * newIndexCount, indexData);
    }
    if (autoUnbind)
        unbind();
    indexCount = newIndexCount;
    primitive = newPrimitive;
}

void GLMesh::destroy()
{
    GLuint buffers[2];
    buffers[0] = vertexBuffer;
    buffers[1] = indexBuffer;
    glDeleteBuffers(2, buffers);
    glDeleteVertexArrays(1, &arrayBuffer);
}

void GLMesh::updateVertexAttributes()
{
    GLuint attrloc;
    GLuint count;
    GLuint stride = GLMeshStride[format];

    const bool autoUnbind = bind();

    count = (format == PTNC) ? 4 : 3;
    //attrloc = glGetAttribLocation(program, "aVertex");
    attrloc = PositionAttribLocation;
    glEnableVertexAttribArray(attrloc);

    glVertexAttribPointer(attrloc, count, GL_FLOAT, GL_FALSE, stride, 0);
    GLuint offset = count << 2;

    if (format >= XYZUV)
    {
        count = (format == PTNC) ? 4 : 2;
        //attrloc = glGetAttribLocation(program, "aTexCoord");
        attrloc = TexCoordAttribLocation;
        glEnableVertexAttribArray(attrloc);
        glVertexAttribPointer(attrloc, count, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += count << 2;
    }

    if (format >= XYZUVN)
    {
        count = 3;
        //attrloc = glGetAttribLocation(program, "aNormal");
        attrloc = NormalAttribLocation;
        glEnableVertexAttribArray(attrloc);
        glVertexAttribPointer(attrloc, count, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        offset += count << 2;
    }

    if (format >= XYZUVNC)
    {
        count = 4;
        //attrloc = glGetAttribLocation(program, "aColor");
        attrloc = ColorAttribLocation;
        glEnableVertexAttribArray(attrloc);
        glVertexAttribPointer(attrloc, count, GL_FLOAT, GL_FALSE, stride, (void*)offset);
    }

    if (autoUnbind)
        unbind();
}

bool GLMesh::bind()
{
    if (isBound)
        return false;
    glBindVertexArray(arrayBuffer);
    isBound = true;
    return true;
}

void GLMesh::unbind()
{
    if (!isBound)
        return;
    glBindVertexArray(0);
    isBound = false;
}

void GLMesh::render()
{
    const bool autoUnbind = bind();
    glDrawElements((primitive == TRIANGLES) ? GL_TRIANGLES : GL_LINES, indexCount, GL_UNSIGNED_INT, 0);
    if (autoUnbind)
        unbind();
}

GLTexture::GLTexture(GLuint topology, GLuint format, GLuint internalFormat, GLuint type,
    GLuint minFilter, GLuint magFilter, GLuint wrapS, GLuint wrapT)
    : topology(topology)
    , format(format)
    , internalFormat(internalFormat)
    , type(type)
    , minFilter(minFilter)
    , magFilter(magFilter)
    , wrapS(wrapS)
    , wrapT(wrapT)
{
    glGenTextures(1, &texture);
}

void GLTexture::destroy()
{
    glDeleteTextures(1, &texture);
}

void GLTexture::updateSettings()
{
    glBindTexture(topology, texture);
    glTexParameteri(topology, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(topology, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(topology, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(topology, GL_TEXTURE_WRAP_T, wrapT);
}

void GLTexture::setTexImage2D(const GLvoid* data, GLuint width, GLuint height, GLuint mipLevel)
{
    assert(topology == GL_TEXTURE_2D);
    glBindTexture(topology, texture);
    glTexImage2D(topology, mipLevel, internalFormat, width, height, 0, format, type, data);
}

void GLTexture::setTexImage3D(const GLvoid* data, GLuint width, GLuint height, GLuint depth, GLuint mipLevel)
{
    assert(topology == GL_TEXTURE_3D);
    glBindTexture(topology, texture);
    glTexImage3D(topology, mipLevel, internalFormat, width, height, depth, 0, format, type, data);
}

void GLTexture::setTexImageCube(const GLvoid* data, GLuint side, GLuint width, GLuint height, GLuint mipLevel)
{
    assert(topology == GL_TEXTURE_CUBE_MAP);
    glBindTexture(topology, texture);
    glTexImage2D(side, mipLevel, internalFormat, width, height, 0, format, type, data);
}

void GLTexture::setTexStorage2D(GLuint width, GLuint height, GLuint levels)
{
    assert(topology == GL_TEXTURE_2D);
    glBindTexture(topology, texture);
    glTexStorage2D(topology, levels, internalFormat, width, height);
}

void GLTexture::generateMipmap()
{
    glBindTexture(topology, texture);
    glGenerateMipmap(topology);
    //glGenerateTextureMipmap(texture);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(topology, texture);
}

void GLTexture::bindUniform(GLuint program, GLuint textureUnit)
{
    char name[10] = "uTexture#";
    name[8] = '0' + textureUnit;
    GLuint uloc = glGetUniformLocation(program, name);
    glUniform1i(uloc, textureUnit);
}

void GLTexture::bind(GLuint program, GLuint textureUnit)
{
    bindUniform(program, textureUnit);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    //glEnable(topology); // this is necessary?
    glBindTexture(topology, texture);
    // TODO: glBindSampler
}

void GLTexture::bindImage(GLuint program, GLuint textureUnit, GLenum access,
    GLuint level, GLboolean layered, GLuint layer)
{
    //bindUniform(program, textureUnit);

    //glActiveTexture(GL_TEXTURE0 + textureUnit);
    //glBindTexture(topology, texture);

    glBindImageTexture(textureUnit, texture, level, layered, layer, access, internalFormat);
}

FileBuffer::FileBuffer(const char* file_name, bool text_mode)
{
    append(file_name, text_mode);
}

void FileBuffer::append(const char* file_name, bool text_mode)
{
    FILE* fp = fopen(file_name, text_mode ? "r" : "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: file %s not found\n", file_name);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto prevSize = buffer.size();
    if (!prevSize)
        ++prevSize;
    buffer.resize(prevSize + size);
    size = fread(buffer.data() + prevSize - 1, 1, size, fp);
    buffer.resize(prevSize + size);
    buffer.back() = 0;

    fclose(fp);
}

void FileBuffer::save(const char* file_name, bool text_mode)
{
    FILE* fp = fopen(file_name, text_mode ? "w" : "wb");
    if (!fp) {
        fprintf(stderr, "ERROR: cannot create file %s\n", file_name);
        return;
    }

    const auto size = buffer.size();
    if (size > 1)
        fwrite(buffer.data(), 1, size - 1, fp);
    fclose(fp);
}

MeshBuilder::MeshBuilder(GLMesh::Format format)
    : format(format)
{
    clear();
}

void MeshBuilder::clear()
{
    currMode = TRIANGLES;
    currNormal = vec3(0, 0, 1);
    currColor = vec4(1);
    currTexCoord = vec4(0);
    currBeginOffset = 0;
    vertexData.resize(0);
    indexData.resize(0);
}

void MeshBuilder::begin(Mode mode)
{
    currMode = mode;
    currBeginOffset = static_cast<GLuint>(vertexData.size());
}

void MeshBuilder::end()
{
    GLuint vertexSize = GLMeshStride[format] >> 2;
    GLuint it = currBeginOffset / vertexSize;
    GLuint itEnd = static_cast<GLuint>(vertexData.size()) / vertexSize;
    assert(it * vertexSize == currBeginOffset);
    assert(itEnd * vertexSize == vertexData.size());
    switch(currMode)
    {
    case LINE_STRIP:
    {
        GLuint prev = it;
        for (++it; it < itEnd; prev = it, ++it) {
            indexData.push_back(prev);
            indexData.push_back(it);
        }
        break;
    }
    case LINE_LOOP:
    {
        GLuint it0 = it, prev = it;
        ++it;
        if (it < itEnd)
        {
            for (; it < itEnd; prev = it, ++it) {
                indexData.push_back(prev);
                indexData.push_back(it);
            }
            indexData.push_back(prev);
            indexData.push_back(it0);
        }
        break;
    }
    case LINES:
    case TRIANGLES:
        for(; it < itEnd; ++it)
            indexData.push_back(it);
        break;
    case TRIANGLE_FAN:
    {
        GLuint it0 = it;
        for(it += 2; it < itEnd; ++it) {
            indexData.push_back(it0);
            indexData.push_back(it - 1);
            indexData.push_back(it);
        }
        break;
    }
    case QUADS:
        for (; it < itEnd; it += 4) {
            // 0 1
            // 3 2
            indexData.push_back(it);
            indexData.push_back(it + 1);
            indexData.push_back(it + 3);
            indexData.push_back(it + 1);
            indexData.push_back(it + 2);
            indexData.push_back(it + 3);
        }
        break;
    }
}

void MeshBuilder::vertex(const vec4& v)
{
    vertexData.push_back(static_cast<float>(v.x));
    vertexData.push_back(static_cast<float>(v.y));
    vertexData.push_back(static_cast<float>(v.z));
    if (format == GLMesh::PTNC) {
        vertexData.push_back(static_cast<float>(v.w));
    }
    if (format >= GLMesh::XYZUV) {
        vertexData.push_back(static_cast<float>(currTexCoord.x));
        vertexData.push_back(static_cast<float>(currTexCoord.y));
    }
    if (format == GLMesh::PTNC) {
        vertexData.push_back(static_cast<float>(currTexCoord.z));
        vertexData.push_back(static_cast<float>(currTexCoord.w));
    }
    if (format >= GLMesh::XYZUVN) {
        vertexData.push_back(static_cast<float>(currNormal.x));
        vertexData.push_back(static_cast<float>(currNormal.y));
        vertexData.push_back(static_cast<float>(currNormal.z));
    }
    if (format >= GLMesh::XYZUVNC) {
        vertexData.push_back(static_cast<float>(currColor.x));
        vertexData.push_back(static_cast<float>(currColor.y));
        vertexData.push_back(static_cast<float>(currColor.z));
        vertexData.push_back(static_cast<float>(currColor.w));
    }
}

void MeshBuilder::vertex(const vec4& v, const vec4& t, const vec3& n, const vec4& c)
{
    vertexData.push_back(static_cast<float>(v.x));
    vertexData.push_back(static_cast<float>(v.y));
    vertexData.push_back(static_cast<float>(v.z));
    if (format == GLMesh::PTNC) {
        vertexData.push_back(static_cast<float>(v.w));
    }
    if (format >= GLMesh::XYZUV) {
        vertexData.push_back(static_cast<float>(t.x));
        vertexData.push_back(static_cast<float>(t.y));
    }
    if (format == GLMesh::PTNC) {
        vertexData.push_back(static_cast<float>(t.z));
        vertexData.push_back(static_cast<float>(t.w));
    }
    if (format >= GLMesh::XYZUVN) {
        vertexData.push_back(static_cast<float>(n.x));
        vertexData.push_back(static_cast<float>(n.y));
        vertexData.push_back(static_cast<float>(n.z));
    }
    if (format >= GLMesh::XYZUVNC) {
        vertexData.push_back(static_cast<float>(c.x));
        vertexData.push_back(static_cast<float>(c.y));
        vertexData.push_back(static_cast<float>(c.z));
        vertexData.push_back(static_cast<float>(c.w));
    }
}

GLMesh MeshBuilder::compile() const
{
    const auto primitive = (static_cast<unsigned>(currMode) <= static_cast<unsigned>(LINE_LOOP))
        ? GLMesh::LINES : GLMesh::TRIANGLES;
    return GLMesh(format, vertexData.data(), vertexData.size(), indexData.data(), indexData.size(),
        primitive);
}

void MeshBuilder::compile(GLMesh& mesh) const
{
    const bool autoUnbind = mesh.bind();
    const auto primitive = (static_cast<unsigned>(currMode) <= static_cast<unsigned>(LINE_LOOP))
        ? GLMesh::LINES : GLMesh::TRIANGLES;
    mesh.updateVertices(format, vertexData.data(), vertexData.size());
    mesh.updateIndices(indexData.data(), indexData.size(), primitive);
    if (autoUnbind)
        mesh.unbind();
}
