// basic error checking for opengl

#ifndef _ERROR_H_
#define _ERROR_H_

#include <glad/glad.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H

#include "log.h"

static char const * gl_error_string(GLenum const err) noexcept {
    switch (err) {
        // opengl 2 errors (8)
    case GL_NO_ERROR:
        return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";

    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";

    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";

    default:
        return "Undefined error";
    }
}

static GLuint gl_error_reserved     = GL_NO_ERROR;
static FT_Error ft_error_reserved   = FT_Err_Ok;

#define GL_CALL(X)\
    X;\
    gl_error_reserved = glGetError();\
    if (gl_error_reserved != GL_NO_ERROR) {\
        LOG_ERROR("OpenGL error: %s [LINE: %d, FILE: %s]", gl_error_string(gl_error_reserved), __LINE__, __FILE__);\
    }\
	
#define FT_CALL(X)\
    ft_error_reserved = X;\
    if (ft_error_reserved != FT_Err_Ok) {\
        LOG_ERROR("FreeType error: %s [LINE: %d, FILE: %s]", FT_Error_String(ft_error_reserved), __LINE__, __FILE__);\
    }
    
#endif //_ERROR_H_
