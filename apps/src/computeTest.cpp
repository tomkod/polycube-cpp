// OpenGL compute example
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
#include <math.h>
#include <assert.h>
#include <stddef.h>

#include <cstdint>

#include <glHelpers.h>

using namespace glsl_math;

static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

GLTexture genMipmappedTextureRGBA8(const void* data, uint32_t width, uint32_t height)
{
    auto ret = GLTexture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE);
    ret.setTexImage2D(data, width, height);
    ret.generateMipmap();
    ret.updateSettings();
    return ret;
}

GLTexture genTextureChecker(uint32_t width, uint32_t height,
    uint32_t radx, uint32_t rady, bool randomizeColors)
{
    std::vector<uint8_t> data(width * height * 4);
    uint8_t color_tab[20] = {
        0x70, 0x80, 0xa0, 0xFF,
        0x70, 0x90, 0x70, 0xFF,
        0x90, 0x90, 0x60, 0xFF,
        0xa0, 0x80, 0x70, 0xFF,
        0xd0, 0xd0, 0xd0, 0xFF
    };
    if (randomizeColors) {
        for (uint32_t k = 0; k < 16; k += 4) {
            color_tab[0] = rand() % 48 + 112;
            color_tab[1] = rand() % 48 + 112;
            color_tab[2] = rand() % 48 + 112;
        }
    }
    for (uint32_t k = 0, y = 0; y < height; y++)
        for (uint32_t x = 0; x < width; x++, k += 4) {
            uint32_t kx = x / radx;
            uint32_t ky = y / rady;
            uint32_t c = (kx + (ky & 1)) & 1;
            if (c) c = (kx * 7 + ky * kx) % 4; else c = 4;
            c <<= 2;
            data[k] = color_tab[c];
            data[k + 1] = color_tab[c + 1];
            data[k + 2] = color_tab[c + 2];
            data[k + 3] = color_tab[c + 3];
        }

    return genMipmappedTextureRGBA8(data.data(), width, height);
}

inline double animFunc(double ang, double anim_time)
{
    return 1.0 + cos(ang*4 + anim_time)*0.1;
}

struct Workspace
{
    std::vector<uint32_t> indexData;
    std::vector<float> vertexData;
};

void genGridIndices(Workspace& wks, GLMesh& mesh, uint32_t res)
{
    wks.indexData.resize(res * res * 6);
    for (uint32_t k = 0, vi = 0, y = 0; y < res; ++y, ++vi)
    {
        for (uint32_t x = 0; x < res; ++x, ++vi, k += 6)
        {
            wks.indexData[k] = vi;
            wks.indexData[k + 1] = vi + 1;
            wks.indexData[k + 2] = vi + res + 1;
            wks.indexData[k + 3] = vi + 1;
            wks.indexData[k + 4] = vi + res + 2;
            wks.indexData[k + 5] = vi + res + 1;
        }
    }
    {
        const bool autoUnbind = mesh.bind();
        mesh.updateIndices(wks.indexData.data(), wks.indexData.size());
        if (autoUnbind)
            mesh.unbind();
    }
}

void genGridVertices(Workspace& wks, GLMesh& mesh, uint32_t res, float time)
{
    uint32_t stride = 4 + 4 + 3 + 4;
    wks.vertexData.resize((res + 1) * (res + 1) * stride);
    for (uint32_t k = 0, vi = 0, y = 0; y <= res; ++y, ++vi)
    {
        const float v = y / static_cast<float>(res);
        for (uint32_t x = 0; x <= res; ++x, ++vi, k += stride)
        {
            const float u = x / static_cast<float>(res);

            wks.vertexData[k] = u * 2 - 1;
            wks.vertexData[k + 1] = v * 2 - 1;
            wks.vertexData[k + 2] = sin(u*8.f + sin(v*8.f + time)) * .3f;
            wks.vertexData[k + 3] = 1;

            wks.vertexData[k + 4] = u;
            wks.vertexData[k + 5] = v;
            wks.vertexData[k + 6] = 0;
            wks.vertexData[k + 7] = 1;

            wks.vertexData[k + 8] = 0;
            wks.vertexData[k + 9] = 0;
            wks.vertexData[k + 10] = 1;

            wks.vertexData[k + 11] = 1;
            wks.vertexData[k + 12] = 1;
            wks.vertexData[k + 13] = 1;
            wks.vertexData[k + 14] = 1;
        }
    }
    {
        const bool autoUnbind = mesh.bind();
        mesh.updateVertices(GLMesh::PTNC, wks.vertexData.data(), wks.vertexData.size());
        if (autoUnbind)
            mesh.unbind();
    }
}

static double mousx;
static double mousy;
static double mousz;
static double omousx;
static double omousy;
static int bstatus = 0;
static int event_keystatus[GLFW_KEY_LAST + 1];
static int keystatus[GLFW_KEY_LAST + 1];

static void mouseButtonCallback(GLFWwindow*, int button, int action, int mods)
{
    int bstatusBit = 0;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        bstatusBit = 1;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        bstatusBit = 2;
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        bstatusBit = 4;
        break;
    }
    switch(action)
    {
        case GLFW_PRESS:
            bstatus |= bstatusBit;
            break;
        case GLFW_RELEASE:
            bstatus &= ~bstatusBit;
            break;
        case GLFW_REPEAT:
            break;
    }
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key < 0 || key > GLFW_KEY_LAST)
        return;

    if (key == GLFW_KEY_ESCAPE)
    {
        // Exit program on Escape.
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    switch (action)
    {
    case GLFW_PRESS:
        event_keystatus[key] = 1;
        keystatus[key] = 1;
        break;
    case GLFW_RELEASE:
        event_keystatus[key] = 0;
        break;
    case GLFW_REPEAT:
        break;
    }
}

static void mousePositionCallback(GLFWwindow*, double x, double y)
{
    mousx = x;
    mousy = y;
}

static void mouseScrollCallback(GLFWwindow*, double x, double)
{
    mousz = x;
}

struct View
{
    vec2 cam_ang;
    vec2 light_ang;
    vec3 cam_pos;
    double cam_dist;
};

void updateCamera(View& view, mat4& cam_mat, vec3& light_dir)
{
    // Simple 4dof rotating camera:
    double f;
    if (bstatus % 4 >= 2) {
        view.cam_dist += view.cam_dist
            * max(min(mousy - omousy, 100.0), -100.0) * .005;
    }
    else if (bstatus) {
        f = 0.006;
        if (keystatus[GLFW_KEY_LEFT_SHIFT] | keystatus[GLFW_KEY_RIGHT_SHIFT]) {
            view.light_ang.x -= (mousx - omousx)*f;
            view.light_ang.y += (mousy - omousy)*f;
            view.light_ang.y = min(M_PI - .1, max(.1, view.light_ang.y));
        }
        else if (keystatus[GLFW_KEY_LEFT_CONTROL] | keystatus[GLFW_KEY_RIGHT_CONTROL]) {
            f *= view.cam_dist*.3;
            const double dx = (mousx - omousx)*f;
            const double dy = (mousy - omousy)*f;
            const double co_x = cos(view.cam_ang.x);
            const double si_x = sin(view.cam_ang.x);
            const double co_y = cos(view.cam_ang.y);
            //if (abs(co_y) > .7071) 
            {
                //dy *= sign(sin(cam_ang.y));
                view.cam_pos.x += co_x*dx + si_x*dy;
                view.cam_pos.y -= co_x*dy - si_x*dx;
            }
            /* else {
            cam_pos.x += co_x*dx;
            cam_pos.y += si_x*dx;
            cam_pos.z -= dy*co_y;
            }*/
        }
        else {
            view.cam_ang.x -= (mousx - omousx)*f;
            view.cam_ang.y += (mousy - omousy)*f;
            view.cam_ang.y = max(-M_PI*.5, min(M_PI*.5, view.cam_ang.y));
        }
    }
    omousx = mousx;
    omousy = mousy;

    identity(cam_mat);
    translate(cam_mat, 0.0, 0.0, -view.cam_dist);
    rotate(cam_mat, -view.cam_ang.x * 180.0 / M_PI, 0.0, 1.0, 0.0);
    rotate(cam_mat, view.cam_ang.y * 180.0 / M_PI, 1.0, 0.0, 0.0);
    translate(cam_mat, view.cam_pos.x, view.cam_pos.y, view.cam_pos.z);

    f = cos(view.light_ang.y);
    light_dir.x = sin(view.light_ang.x)*f;
    light_dir.y = cos(view.light_ang.x)*f;
    light_dir.z = sin(view.light_ang.y);
}

int main(int argc, char** argv)
{
#if GlslMathUnitTests
    runUnitTests();
#endif

    GLFWwindow* window;
    double curr_time;
    double prev_time;
    double delta_time;
    int frame;

    View view;
    view.cam_ang = vec2(0, 0);
    view.light_ang = vec2(0,.8);
    view.cam_pos = vec3(0);
    view.cam_dist = 3.6;

    GLuint shader_program;

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    window = glfwCreateWindow(1280, 720, "GLFW OpenGL3 Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Register events callback.
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePositionCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Prepare opengl resources for rendering.
    {
        FileBuffer vertex_shader("ptnc.vert", true);
        FileBuffer fragment_shader("textured.frag", true);
        shader_program = compileShaderProgram(vertex_shader.buffer.data(), fragment_shader.buffer.data());
    }

    if (shader_program == 0u)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    const uint32_t grid_res = 127;
    const uint32_t grid_vertex_count = (grid_res + 1) * (grid_res + 1);
    const uint32_t grid_quads_count = grid_res * grid_res;

    GLMesh grid_mesh;

#if GLComputeSupported
    #define UseIndirect 1

    GLuint tex_compute_program;
    GLuint geom_compute_program;
    GLuint draw_compute_program;

    {
        FileBuffer compute_shader("gentex.comp", true);
        tex_compute_program = compileComputeProgram(compute_shader.buffer.data());
    }

    if (tex_compute_program == 0u)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    {
        FileBuffer compute_shader("gengrid.comp", true);
        geom_compute_program = compileComputeProgram(compute_shader.buffer.data());
    }

    if (geom_compute_program == 0u)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    {
        FileBuffer compute_shader("gendraw.comp", true);
        draw_compute_program = compileComputeProgram(compute_shader.buffer.data());
    }

    if (draw_compute_program == 0u)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

  #if UseIndirect
    GLuint acbo;
    glGenBuffers(1, &acbo);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_DRAW);
        // GL_DYNAMIC_DRAW and not GL_DYNAMIC_COPY since we reset it using glBufferSubData
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

    GLuint cmdbo;
    glGenBuffers(1, &cmdbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cmdbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 5, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  #endif

    glUseProgram(tex_compute_program);

    GLTexture texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE,
        GLTexture::defMinFilter, GLTexture::defMagFilter, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    texture.setTexStorage2D(128, 128, 1);
    texture.updateSettings();
    texture.bindImage(tex_compute_program);

    glDispatchCompute(128 / 16, 128 / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    texture.generateMipmap();

    grid_mesh.initComputeVertices(GLMesh::PTNC, grid_vertex_count);
    grid_mesh.initComputeIndices(grid_quads_count * 6);

    glUseProgram(geom_compute_program);
    GLuint uTime = glGetUniformLocation(geom_compute_program, "uTime");

    {
        glUseProgram(draw_compute_program);
        GLuint uPrimitiveSize = glGetUniformLocation(draw_compute_program, "uPrimitiveSize");
        glUniform1ui(uPrimitiveSize, 6u);
    }
#else
    #define UseIndirect 0
    GLTexture texture = genTextureChecker(128, 128, 32, 32, false);
    Workspace wks;
    genGridIndices(wks, grid_mesh, grid_res);
#endif

    glUseProgram(shader_program);

    texture.bind(shader_program);

    GLuint uLightDir = glGetUniformLocation(shader_program, "uLightDir");
    GLuint uColor = glGetUniformLocation(shader_program, "uColor");

    int frameWidth, frameHeight;
    glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
    const mat4 projection =
        perspectiveProjection(90.0, frameWidth/static_cast<double>(frameHeight), 0.1, 100.0);
    setProjectionMatrix(shader_program, projection);

    // Create mesh data.
    GLMesh mesh;

    // Setup the scene ready for rendering.
    glViewport(0, 0, frameWidth, frameHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    frame = 0;
    curr_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        ++frame;

        prev_time = curr_time;
        curr_time = glfwGetTime();
        delta_time = curr_time - prev_time;

        mat4 modelView;
        vec3 lightDir;
        updateCamera(view, modelView, lightDir);

        glUseProgram(shader_program);

        setUniformf(uLightDir, lightDir);
        setUniformf(uColor, vec4f(1));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        setModelViewMatrix(shader_program, modelView, true);

#if GLComputeSupported
    #if UseIndirect
        // Reset quad counter
        {
            //glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo);
            GLuint quadCounter = 0;
            glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &quadCounter); // reset quad counter
            //*(GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0,
            //    sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT) = 0;
            //glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        }
    #endif

        glUseProgram(geom_compute_program);
        glUniform1f(uTime, static_cast<float>(curr_time));
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, grid_mesh.vertexBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_mesh.indexBuffer);
    #if UseIndirect
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, acbo);
    #endif
        glDispatchCompute(grid_vertex_count / 64, 1, 1);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

    #if UseIndirect
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);

        glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);

        glUseProgram(draw_compute_program);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cmdbo);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, acbo);
        glDispatchCompute(1, 1, 1);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, 0);
    #endif

        glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT
            | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT);

    #if 0
        // Read atomic
        {
            glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT);
            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, acbo);
            GLuint quadCounter;
            glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &quadCounter);
            glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
        }
    #endif
#else
        genGridVertices(wks, grid_mesh, grid_res, curr_time);
#endif

        glUseProgram(shader_program);
#if UseIndirect
        {
            const bool autoUnbind = grid_mesh.bind();
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdbo);
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
            if (autoUnbind)
                grid_mesh.unbind();
        }
#else
        grid_mesh.render();
#endif

        // Display and process events through callbacks.
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!validateGL())
            break;


        // Reset released keys at the end of frame to not miss any keystroke.
        for (int key = 0; key <= GLFW_KEY_LAST; ++key)
            if (!event_keystatus[key])
                keystatus[key] = 0;
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
