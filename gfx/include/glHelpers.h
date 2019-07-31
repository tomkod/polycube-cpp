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

#pragma once

#include <glslMath.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef GL_COMPUTE_SHADER
#define GLComputeSupported 1
#else
#define GLComputeSupported 0
#endif

#include <vector>

bool validateGL();

GLuint compileShader(GLenum type, const char* text);

GLuint compileShaderProgram(const char* vs_text, const char* fs_text);

#if GLComputeSupported
GLuint compileComputeProgram(const char* cs_text);
#endif

void setUniformf(GLuint uloc, const glsl_math::vec2& v);
void setUniformf(GLuint uloc, const glsl_math::vec3& v);
void setUniformf(GLuint uloc, const glsl_math::vec4& v);
void setUniformf(GLuint uloc, const glsl_math::mat2& m);
void setUniformf(GLuint uloc, const glsl_math::mat3& m);
void setUniformf(GLuint uloc, const glsl_math::mat4& m);

void setProjectionMatrix(GLuint program, const glsl_math::mat4& m);

void setModelViewMatrix(GLuint program, const glsl_math::mat4& m, bool normal_matrix);

class GLMesh
{
public:
    enum Format
    {
        None = 0,
        XYZ, // 12
        XYZUV, // 12 + 8
        XYZUVN, // 12 + 8 + 12
        XYZUVNC, // 12 + 8 + 12 + 16
        PTNC // 16 + 16 + 12 + 16 (4d pos, 4d tex, 3d norm, 4d color)
    };
    enum Primitive
    {
        LINES = 0,
        TRIANGLES
    };
    GLMesh();
    GLMesh(Format format, GLuint vertexCount, GLuint indexCount, Primitive primitive = TRIANGLES);
    GLMesh(Format format, const GLfloat* vertexData, GLuint vertexCount,
        const GLuint* indexData, GLuint indexCount, Primitive primitive = TRIANGLES);

#if GLComputeSupported
    void initComputeVertices(Format newFormat, GLuint newVertexCount);
    void initComputeIndices(GLuint newIndexCount);
#endif

    void updateVertices(Format format, const GLfloat* vertexData, GLuint vertexCount);

    void updateIndices(const GLuint* indexData, GLuint indexCount, Primitive primitive = TRIANGLES);

    void destroy();

    void updateVertexAttributes();

    bool bind();

    void unbind();

    void render();

    Format format;
    Primitive primitive;
    GLuint vertexCount;
    GLuint vertexBuffer;
    GLuint indexCount;
    GLuint indexBuffer;
    GLuint arrayBuffer;
    bool isBound;
};

class GLTexture
{
public:
    static constexpr GLuint defMinFilter = GL_LINEAR_MIPMAP_LINEAR;
    static constexpr GLuint defMagFilter = GL_LINEAR;
    static constexpr GLuint defWrapS = GL_REPEAT;
    static constexpr GLuint defWrapT = GL_REPEAT;
    /**
     * topology:
     *  GL_TEXTURE_2D
     *  GL_TEXTURE_3D
     *  GL_TEXTURE_CUBE_MAP
     * format:
     *  GL_RED
     *  GL_RED_INTEGER
     *  GL_RG
     *  GL_RG_INTEGER
     *  GL_RGB
     *  GL_RGB_INTEGER
     *  GL_RGBA
     *  GL_RGBA_INTEGER
     *  GL_LUMINANCE
     *  GL_LUMINANCE_ALPHA
     *  GL_ALPHA
     *  GL_DEPTH_COMPONENT
     *  GL_DEPTH_STENCIL
     * internalFormat:
     *  GL_R8
     *  GL_RGBA8
     *  GL_SRGB8_ALPHA8
     *  etc..
     *  see https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml table
     * type:
     *  GL_BYTE
     *  GL_SHORT
     *  GL_INT
     *  GL_UNSIGNED_BYTE
     *  GL_UNSIGNED_SHORT
     *  GL_UNSIGNED_INT
     *  GL_UNSIGNED_INT_24_8
     *  GL_HALF_FLOAT
     *  GL_FLOAT
     */
    GLTexture(GLuint topology, GLuint format, GLuint internalFormat, GLuint type,
        GLuint minFilter = defMinFilter, GLuint magFilter = defMagFilter,
        GLuint wrapS = defWrapS, GLuint wrapT = defWrapT);
    
    void destroy();

    void setTexImage2D(const GLvoid* data, GLuint width, GLuint height, GLuint mipLevel = 0);
    void setTexImage3D(const GLvoid* data, GLuint width, GLuint height, GLuint depth, GLuint mipLevel = 0);

    /**
     * side:
     *  GL_TEXTURE_CUBE_MAP_POSITIVE_X
     *  GL_TEXTURE_CUBE_MAP_NEGATIVE_X
     *  GL_TEXTURE_CUBE_MAP_POSITIVE_Y
     *  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
     *  GL_TEXTURE_CUBE_MAP_POSITIVE_Z
     *  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
     */
    void setTexImageCube(const GLvoid* data, GLuint side, GLuint width, GLuint height, GLuint mipLevel = 0);

    void setTexStorage2D(GLuint width, GLuint height, GLuint levels);

    void updateSettings();

    void generateMipmap();

    void bindUniform(GLuint program, GLuint textureUnit);

    void bind(GLuint program, GLuint textureUnit = 0);

    void bindImage(GLuint program, GLuint textureUnit = 0, GLenum access = GL_WRITE_ONLY,
        GLuint level = 0, GLboolean layered = false, GLuint layer = 0);

    GLuint topology;
    GLuint format;
    GLuint internalFormat;
    GLuint type;
    GLuint minFilter;
    GLuint magFilter;
    GLuint wrapS;
    GLuint wrapT;
    GLuint texture;
};

class FileBuffer
{
public:
    FileBuffer(const char* file_name, bool text_mode);
    void append(const char* file_name, bool text_mode);
    void save(const char* file_name, bool text_mode);
    std::vector<char> buffer;
};

class MeshBuilder
{
public:
    using vec2 = glsl_math::vec2;
    using vec3 = glsl_math::vec3;
    using vec4 = glsl_math::vec4;

    enum Mode
    {
        LINES,
        LINE_STRIP,
        LINE_LOOP,
        TRIANGLES,
        //TRIANGLE_STRIP,
        TRIANGLE_FAN,
        QUADS
    };

    MeshBuilder(GLMesh::Format format);

    void clear();
    void begin(Mode);
    void end();
    inline void normal(const vec3& n) { currNormal = n; }
    inline void normal(double x, double y, double z) { currNormal = vec3(x, y, z); }
    inline void texCoord(const vec2& t) { currTexCoord = t; }
    inline void texCoord(const vec4& t) { currTexCoord = t; }
    inline void texCoord(double x, double y) { currTexCoord = vec2(x, y); }
    inline void texCoord(double x, double y, double z, double w) { currTexCoord = vec4(x, y, z, w); }
    inline void color(const vec3& c) { currColor = c; }
    inline void color(const vec4& c) { currColor = c; }
    inline void color(double x, double y, double z) { currColor = vec3(x, y, z); }
    inline void color(double x, double y, double z, double w) { currColor = vec4(x, y, z, w); }
    void vertex(const vec4& v);
    inline void vertex(const vec2& v) { vertex(vec4(v)); }
    inline void vertex(const vec3& v) { vertex(vec4(v)); }
    inline void vertex(double x, double y) { vertex(vec4(x, y, 0, 1)); }
    inline void vertex(double x, double y, double z) { vertex(vec4(x, y, z, 1)); }
    inline void vertex(double x, double y, double z, double w) { vertex(vec4(x, y, z, w)); }
    void vertex(const vec4& v, const vec4& t, const vec3& n, const vec4& c);

    GLMesh compile() const;
    void compile(GLMesh& mesh) const;

    GLMesh::Format format;

    Mode currMode;
    vec3 currNormal;
    vec4 currTexCoord;
    vec4 currColor;
    GLuint currBeginOffset;

    std::vector<GLfloat> vertexData;
    std::vector<GLuint> indexData;
};
