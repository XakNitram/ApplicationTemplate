#include "lwvl/lwvl.hpp"

namespace lwvl {
    GLuint Texture::reserve(GLenum type) {
        GLuint temp;
        glCreateTextures(type, 1, &temp);
        return temp;
    }

    Texture::Texture(GLenum type) : id(reserve(type)) {}

    Texture::Texture(GLenum, GLuint id) : id(id) {}

    Texture::operator GLuint() const {
        return id;
    }

    Texture::Texture(lwvl::Texture &&other) noexcept {
        glDeleteTextures(1, &id);
        id = other.id;
        other.id = 0;
    }

    Texture &Texture::operator=(lwvl::Texture &&other) noexcept {
        glDeleteTextures(1, &id);
        id = other.id;
        other.id = 0;
        return *this;
    }

    Texture::~Texture() {
        glDeleteTextures(1, &id);
    }
}
