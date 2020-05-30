
/*
 * handle the creation of opengl objects (textures, shaders, etc)
 * also takes care of rendering and show different framebuffers
 */

#ifndef _OPENGLMANAGER_H_
#define _OPENGLMANAGER_H_

#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

#include "texture.h"

class opengl_manager {
public:
    enum shaders {
        DEFAULT_SHADER,
        SDF_SHADER
    };

    enum framebuffers {
        DEFAULT_FBO,
        SDF_FBO
    };
public:
    // initialize all required objects and create shaders, framebuffers
    // the width and height are for the viewport and not the screen
    static bool init(int width, int height);
    // delete the objects
    static void destroy();

    // dimen and pos in pixels
    static void draw(texture * tex, glm::vec2 position, glm::vec2 dimen, shaders shader, framebuffers fbo);

    // show the different framebuffers
    static void gui();

    // clear fbo specified by fbo_index
    static void clear_fbo(framebuffers fbo_index);
private:
    // create a shader program from shader codes
    static GLuint create_shader_program(const GLchar * vertex_shader_code, const GLchar * fragment_shader_code);
private:
    // opengl specific objects
    static GLuint vertex_buffer;
    static GLuint vertex_array;
    static std::unordered_map<shaders, GLuint> shader_programs;
    // pair of fbo objects and it's texture
    static std::unordered_map<framebuffers, std::pair<GLuint, texture *>> fbos;

    // misc data
    static int viewport_width, viewport_height;

    // matrices
    static glm::mat4 projection;

    // params
    static float zoom;
    static float width;
    static float edge;
    static glm::vec2 offset;
    static bool show_rendered;
};

#endif //_OPENGLMANAGER_H_