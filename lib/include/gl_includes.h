#ifndef __GL_INCLUDES_H__
#define __GL_INCLUDES_H__

#if defined(__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
#elif defined(WIN32)
    #define GLEW_STATIC
    #include <GL/glew.h>
#else
    #define GL_GLEXT_PROTOTYPES
#endif

#include <GLFW/glfw3.h>
namespace konstructs {
    template <typename T> struct type_traits;
    template <> struct type_traits<uint32_t> { enum { type = GL_UNSIGNED_INT, integral = 1 }; };
    template <> struct type_traits<int32_t> { enum { type = GL_INT, integral = 1 }; };
    template <> struct type_traits<uint16_t> { enum { type = GL_UNSIGNED_SHORT, integral = 1 }; };
    template <> struct type_traits<int16_t> { enum { type = GL_SHORT, integral = 1 }; };
    template <> struct type_traits<uint8_t> { enum { type = GL_UNSIGNED_BYTE, integral = 1 }; };
    template <> struct type_traits<int8_t> { enum { type = GL_BYTE, integral = 1 }; };
    template <> struct type_traits<double> { enum { type = GL_DOUBLE, integral = 0 }; };
    template <> struct type_traits<float> { enum { type = GL_FLOAT, integral = 0 }; };
};
#endif
