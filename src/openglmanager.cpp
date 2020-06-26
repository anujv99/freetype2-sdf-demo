#include "openglmanager.h"

#include <imgui.h>

#include "error.h"

// ------------------declarations------------------
GLuint opengl_manager::vertex_buffer;
GLuint opengl_manager::vertex_array;
std::unordered_map<opengl_manager::shaders, GLuint> opengl_manager::shader_programs;
std::unordered_map<opengl_manager::framebuffers, std::pair<GLuint, texture *>> opengl_manager::fbos;

int opengl_manager::viewport_width		= 0;
int opengl_manager::viewport_height		= 0;
glm::mat4 opengl_manager::projection	= glm::mat4(1.0f);
float opengl_manager::zoom				= 1.0f;
glm::vec2 opengl_manager::offset        = glm::vec2(0.0f);
bool opengl_manager::show_rendered      = false;

static float swidth = 0.0f;
static float sedge  = 0.1f;
// ------------------------------------------------

static bool check_shader_compile_status(GLuint shader) {
	int success;
	GLchar info_log[512];
	GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

	if (!success) {
		GL_CALL(glGetShaderInfoLog(shader, 512, NULL, info_log));
		LOG_ERROR("shader compilation failed: %s", info_log);
		return false;
	}
	return true;
}

bool opengl_manager::init(int width, int height) {
	if (width <= 0 || height <= 0) {
		LOG_ERROR("invalid window size");
		return false;
	}
	viewport_width = width;
	viewport_height = height;
	// ----------------- create vertex buffer -----------------
	// first 2 float = position
	// last 2 float = texture coordinate
	static float vertices[] = {
		-0.5f,  0.5f, 0.0f, 1.0f,	// top-left
		-0.5f, -0.5f, 0.0f, 0.0f,	// bottom-left
		 0.5f, -0.5f, 1.0f, 0.0f,	// bottom-right

		-0.5f,  0.5f, 0.0f, 1.0f,	// top-left
		 0.5f, -0.5f, 1.0f, 0.0f,	// bottom-right
		 0.5f,  0.5f, 1.0f, 1.0f	// top-right
	};

	// create opengl vertex buffer object to hold the vertices
	GLuint vbo;
	GL_CALL(glGenBuffers(1, &vbo));
	vertex_buffer = vbo;
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), (const void *)vertices, GL_STATIC_DRAW));
	// unbind the buffer
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));

	// ----------------- create shaders -----------------
	GLuint default_vertex_shader, default_fragment_shader;

	// shader codes
	const GLchar * default_vertex_shader_code = R"(
		#version 330 core
		layout (location = 0) in vec2 in_pos;
		layout (location = 1) in vec2 in_tex_coords;

		uniform mat4 projection;
		uniform mat4 model;

		out vec2 pass_tex_coords;

		void main() {
			pass_tex_coords = in_tex_coords;
			gl_Position = projection * model * vec4(in_pos.x, in_pos.y, 0.0f, 1.0f);
		}
	)";

	const GLchar * default_fragment_shader_code = R"(
		#version 330 core
		out vec4 frag_color;

		uniform sampler2D tex; 

		in vec2 pass_tex_coords;

		void main() {
			float color = texture(tex, pass_tex_coords).r;

			frag_color = vec4(color < 0 ? -color : color);
			frag_color.a = 1.0f;
		}
	)";

	const GLchar * sdf_fragment_shader_code = R"(
		#version 330 core
		out vec4 frag_color;

		uniform sampler2D tex;
		uniform bool show_rendered;

		in vec2 pass_tex_coords;

		uniform float width = 0.00f;
		uniform float edge  = 0.01f;

		void main() {
			float distance = texture(tex, pass_tex_coords).r;
			if (!show_rendered) {
				frag_color = vec4(1.0f - abs(distance));
				frag_color.a = 1.0f;
			} else {
				{
					float alpha = 1.0f - smoothstep(width, width + edge, -distance);
					frag_color = vec4(alpha);
					frag_color.a = 1.0f;
				}
			}
		}
	)";

	// create the actual opengl shader object
	GLuint default_shader_program = create_shader_program(default_vertex_shader_code, default_fragment_shader_code);
	if (!default_shader_program) return false;

	GLuint sdf_shader_program = create_shader_program(default_vertex_shader_code, sdf_fragment_shader_code);
	if (!sdf_shader_program) return false;

	shader_programs.insert(std::make_pair(shaders::DEFAULT_SHADER, default_shader_program));
	shader_programs.insert(std::make_pair(shaders::SDF_SHADER, sdf_shader_program));

	// ----------------- create vertex arrays -----------------
	GLuint vao;
	GL_CALL(glGenVertexArrays(1, &vao));
	vertex_array = vao;
	GL_CALL(glBindVertexArray(vao));
	
	// bind buffer so that the vao can cature it
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbo));

	// point to the vbo and set vbo layout
	GL_CALL(glVertexAttribPointer(0u, 2u, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)0));
	GL_CALL(glVertexAttribPointer(1u, 2u, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(2 * sizeof(float))));
	GL_CALL(glEnableVertexAttribArray(0u));
	GL_CALL(glEnableVertexAttribArray(1u));

	// unbind the objects
	GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0u));
	GL_CALL(glBindVertexArray(0u));

	// ----------------- create framebuffers -----------------

	// DEFAULT FRAMEBFUFER
	GLuint default_fbo;
	GL_CALL(glGenFramebuffers(1, &default_fbo));

	// the two params (i.e. format and type) are ignored because the buffer is NULL
	texture * default_fbo_texture = new texture(NULL, viewport_width, viewport_height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR);
	fbos.insert(std::make_pair(framebuffers::DEFAULT_FBO, std::make_pair(default_fbo, default_fbo_texture)));

	// attach the texture to the fbo
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, default_fbo));
	GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, default_fbo_texture->get_id(), 0));

	// check the fbo status
	GLenum status = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOG_ERROR("failed to create framebuffer");
	}

	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	// -------------------------------------------------------

	// SDF FRAMEBFUFER
	GLuint sdf_fbo;
	GL_CALL(glGenFramebuffers(1, &sdf_fbo));

	// the two params (i.e. format and type) are ignored because the buffer is NULL
	texture * sdf_fbo_texture = new texture(NULL, viewport_width, viewport_height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR);
	fbos.insert(std::make_pair(framebuffers::SDF_FBO, std::make_pair(sdf_fbo, sdf_fbo_texture)));

	// attach the texture to the fbo
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, sdf_fbo));
	GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sdf_fbo_texture->get_id(), 0));

	// check the fbo status
	status = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		LOG_ERROR("failed to create framebuffer");
	}

	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	// -------------------------------------------------------

	// set the projection matrix
	projection = glm::ortho(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f);

	return true;
}

void opengl_manager::destroy() {
	GL_CALL(glDeleteBuffers(1, &vertex_buffer));
	GL_CALL(glDeleteVertexArrays(1, &vertex_array));

	for (auto & shader : shader_programs) {
		GL_CALL(glDeleteProgram(shader.second));
		shader.second = 0;
	}

	for (auto & fbo : fbos) {
		GL_CALL(glDeleteFramebuffers(1, &(fbo.second.first)));
		delete fbo.second.second;
		fbo.second.first = 0;
		fbo.second.second = nullptr;
	}
}

void opengl_manager::draw(texture * tex, glm::vec2 position, glm::vec2 dimen, shaders shader, framebuffers fbo) {
	// construct the model matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position + offset, 0.0f));
	model = glm::scale(model, glm::vec3(dimen * zoom, 1.0f));

	// maybe cache the locations?
	GL_CALL(glUseProgram(shader_programs[shader]));
	GLuint model_uniform_location = GL_CALL(glGetUniformLocation(shader_programs[shader], "model"));
	GLuint projection_uniform_location = GL_CALL(glGetUniformLocation(shader_programs[shader], "projection"));

	// bind the fbo
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fbos[fbo].first));

	// set the viewport
	GL_CALL(glViewport(0, 0, fbos[fbo].second->get_width(), fbos[fbo].second->get_height()));

	// set the uniforms
	GL_CALL(glUniformMatrix4fv(model_uniform_location, 1u, GL_FALSE, glm::value_ptr(model)));
	GL_CALL(glUniformMatrix4fv(projection_uniform_location, 1u, GL_FALSE, glm::value_ptr(projection)));

	// bind the texture
	GL_CALL(glBindTexture(GL_TEXTURE_2D, tex->get_id()));

	// bind the vertex array
	GL_CALL(glBindVertexArray(vertex_array));

	// draw call
	GL_CALL(glDrawArrays(GL_TRIANGLES, 0u, 6u));

	// unbind the objects
	GL_CALL(glBindTexture(GL_TEXTURE_2D, 0u));
	GL_CALL(glUseProgram(0u));
	GL_CALL(glBindVertexArray(0u));
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0u));
}

void opengl_manager::gui() {
	ImGuiIO & io = ImGui::GetIO();

	if (ImGui::Begin("Default Framebuffer", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		ImGui::Text("Scroll to zoom");
		ImGui::Text("Hold right click to drag");
		ImGui::Separator();

		texture * default_texture = fbos[DEFAULT_FBO].second;
		ImGui::Image((ImTextureID)(default_texture->get_id()),
					 ImVec2(default_texture->get_width(), default_texture->get_height()));

		if (ImGui::IsWindowHovered()) {
			if (io.MouseWheel > 0)
				zoom += 1.0f;
			else if (io.MouseWheel < 0)
				zoom -= 1.0f;
			if (zoom < 1.0f) zoom = 1.0f;

			if (io.MouseDown[1]) {
				offset += glm::vec2(io.MouseDelta.x, io.MouseDelta.y);
			}
		}
	}
	ImGui::End();

	if (ImGui::Begin("SDF Framebuffer", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		if (ImGui::Checkbox("Show Rendererd", &show_rendered)) {
			GL_CALL(glUseProgram(shader_programs[SDF_SHADER]));
			GLuint location = GL_CALL(glGetUniformLocation(shader_programs[SDF_SHADER], "show_rendered"));

			if (show_rendered) {
				GL_CALL(glUniform1i(location, show_rendered));
			} else {
				GL_CALL(glUniform1i(location, show_rendered));
			}

			GL_CALL(glUseProgram(0));
		}

		if (ImGui::DragFloat("Width", &swidth, 0.01f, 0.0f)) {
			GL_CALL(glUseProgram(shader_programs[SDF_SHADER]));
			GLuint location = GL_CALL(glGetUniformLocation(shader_programs[SDF_SHADER], "width"));

			GL_CALL(glUniform1f(location, swidth));
			GL_CALL(glUseProgram(0));
		}

		if (ImGui::DragFloat("Edge", &sedge, 0.01f, 0.0f)) {
			GL_CALL(glUseProgram(shader_programs[SDF_SHADER]));
			GLuint location = GL_CALL(glGetUniformLocation(shader_programs[SDF_SHADER], "edge"));

			GL_CALL(glUniform1f(location, sedge));
			GL_CALL(glUseProgram(0));
		}

		texture * default_texture = fbos[SDF_FBO].second;
		ImGui::Image((ImTextureID)(default_texture->get_id()),
					 ImVec2(default_texture->get_width(), default_texture->get_height()));

		if (ImGui::IsWindowHovered()) {
			if (io.MouseWheel > 0)
				zoom += 1.0f;
			else if (io.MouseWheel < 0)
				zoom -= 1.0f;
			if (zoom < 1.0f) zoom = 1.0f;

			if (io.MouseDown[1]) {
				offset += glm::vec2(io.MouseDelta.x, io.MouseDelta.y);
			}
		}
	}
	ImGui::End();
}

void opengl_manager::clear_fbo(framebuffers fbo_index) {
	// bind the fbo
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fbos[fbo_index].first));

	// clear the buffer
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

	// unbind the fbo
	GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

GLuint opengl_manager::create_shader_program(const GLchar * vertex_shader_code, const GLchar * fragment_shader_code) {
	glDisable(GL_BLEND);

	// vertex shader
	GLuint vertex_shader = GL_CALL(glCreateShader(GL_VERTEX_SHADER));
	if (vertex_shader == 0u) {
		LOG_ERROR("failed to create vertex shader");
		return 0;
	}
	GL_CALL(glShaderSource(vertex_shader, 1u, &vertex_shader_code, NULL));
	GL_CALL(glCompileShader(vertex_shader));
	if (!check_shader_compile_status(vertex_shader)) {
		GL_CALL(glDeleteShader(vertex_shader));
		return 0;
	}

	// fragment shader
	GLuint fragment_shader = GL_CALL(glCreateShader(GL_FRAGMENT_SHADER));
	if (fragment_shader == 0u) {
		LOG_ERROR("failed to create fragment shader");
		return 0;
	}
	GL_CALL(glShaderSource(fragment_shader, 1u, &fragment_shader_code, NULL));
	GL_CALL(glCompileShader(fragment_shader));
	if (!check_shader_compile_status(fragment_shader)) {
		GL_CALL(glDeleteShader(vertex_shader));
		GL_CALL(glDeleteShader(fragment_shader));
		return 0;
	}

	GLuint shader_program = glCreateProgram();
	if (fragment_shader == 0u) {
		LOG_ERROR("failed to create shader program");
		GL_CALL(glDeleteShader(vertex_shader));
		GL_CALL(glDeleteShader(fragment_shader));
		return 0;
	}

	GL_CALL(glAttachShader(shader_program, vertex_shader));
	GL_CALL(glAttachShader(shader_program, fragment_shader));
	GL_CALL(glLinkProgram(shader_program));

	int success;
	GLchar info_log[512];
	GL_CALL(glGetProgramiv(shader_program, GL_LINK_STATUS, &success));

	if (!success) {
		GL_CALL(glGetProgramInfoLog(shader_program, 512, NULL, info_log));
		LOG_ERROR("program linking failed: %s", info_log);
		GL_CALL(glDeleteShader(vertex_shader));
		GL_CALL(glDeleteShader(fragment_shader));
		GL_CALL(glDeleteProgram(shader_program));
		return 0;
	}

	GL_CALL(glDeleteShader(vertex_shader));
	GL_CALL(glDeleteShader(fragment_shader));
	return shader_program;
}
