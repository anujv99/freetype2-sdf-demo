
#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <glad/glad.h>

class texture {
public:
	/*
	 * buffer can be NULL to create empty texture
	 * internal_format = format of data inside the shader
	 * format = format of data passed to the buffer
	 * type = type of data passed to the buffer
	 * default wrapping is clamp
	 * default filtering is linear
	 */
	texture(const void * buffer, int width, int height, GLuint internal_format, GLuint format, GLuint type, GLuint filtering);
	~texture();

	inline GLuint get_width() const { return m_width; }
	inline GLuint get_height() const { return m_height; }
	inline GLuint get_id() const { return m_texture_id; }
private:
	GLuint m_texture_id;
	GLuint m_internal_format, m_format, m_type;
	GLuint m_width, m_height;
};

#endif //_TEXTURE_H_