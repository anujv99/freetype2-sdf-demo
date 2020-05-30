#include "texture.h"

#include "error.h"

texture::texture(const void * buffer, int width, int height, GLuint internal_format, GLuint format, GLuint type, GLuint filtering) :
	m_texture_id(GL_INVALID_ENUM), m_format(format), m_internal_format(internal_format), m_type(type),
	m_width(width), m_height(height) {

	if (width <= 0 || height <= 0) {
		LOG_ERROR("invalid width/height to create texture");
		return;
	}

	GL_CALL(glGenTextures(1, &m_texture_id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, m_texture_id));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering));

	// Disable byte-alignment restriction
	GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, GL_TRUE));

	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, m_internal_format, width, height, 0, m_format, m_type, buffer));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0u));
}

texture::~texture() {
	GL_CALL(glDeleteTextures(1, &m_texture_id));
}
