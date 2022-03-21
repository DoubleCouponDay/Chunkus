#include "gl.h"


// sauce: stackoverflow
char const* glErrorString(GLenum const err) noexcept
{
    switch (err)
    {
        // opengl 2 errors (8)
    case GL_NO_ERROR:
        return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";

    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";

    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";

    //case GL_TABLE_TOO_LARGE:
    //    return "GL_TABLE_TOO_LARGE";

    //    // opengl 3 errors (1)
    //case GL_INVALID_FRAMEBUFFER_OPERATION:
    //    return "GL_INVALID_FRAMEBUFFER_OPERATION";

        // gles 2, 3 and gl 4 error are handled by the switch above
    default:
        return "UNKNOWN_ERROR";
    }
}

bool checkForGlError(std::string description)
{
    auto err = glGetError();

    if (err != GL_NO_ERROR)
    {
        auto complete_description = description + ": " + glErrorString(err);
        throw GLException(complete_description);
    }
}