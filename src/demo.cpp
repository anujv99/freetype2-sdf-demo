#include "demo.h"

#include <sdfgen.h>
#include <cfloat>
#include <imgui.h>

#include "error.h"
#include "openglmanager.h"

// fonts
#include "roboto.inl"
#include "gkaiu59.inl"

// ------------------declarations------------------
FT_Library demo::library		= nullptr;
FT_Face demo::face				= nullptr;
texture * demo::default_tex		= nullptr;
texture * demo::sdf_tex			= nullptr;

int demo::glyph_index			= 0;
int demo::pixel_size			= 125;
float demo::spread              = 10.0f;
// ------------------------------------------------

static int x[2] = { 0, 0 };

void demo::init() {
	FT_CALL(FT_Init_FreeType(&library));
	//FT_CALL(FT_New_Memory_Face(library, GKAIU59, GKAIU59_SIZE, 0, &face));
	FT_CALL(FT_New_Memory_Face(library, ROBOTO_REGULAR, ROBOTO_REGULAR_SIZE, 0, &face));
	FT_CALL(FT_Set_Pixel_Sizes(face, pixel_size, 0));
	FT_CALL(FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER));

	FT_Bitmap sdf;
	FT_Bitmap_Init(&sdf);

	Generate_SDF(library, face->glyph, &sdf);

	sdf_tex = new texture(sdf.buffer, sdf.width, sdf.rows, GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR);
	default_tex = new texture(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RGBA, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR);

	FT_Bitmap_Done( library, &sdf );
}

void demo::update() {
	opengl_manager::clear_fbo(opengl_manager::SDF_FBO);
	opengl_manager::clear_fbo(opengl_manager::DEFAULT_FBO);

	opengl_manager::draw(sdf_tex, glm::vec2(0.0f), glm::vec2(sdf_tex->get_width(), sdf_tex->get_height()),
		opengl_manager::SDF_SHADER, opengl_manager::SDF_FBO);

	opengl_manager::draw(default_tex, glm::vec2(0.0f), glm::vec2(default_tex->get_width(), default_tex->get_height()),
		opengl_manager::DEFAULT_SHADER, opengl_manager::DEFAULT_FBO);
}

void demo::gui() {
	if (ImGui::Begin("Properties", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		if (ImGui::InputInt("Glyph Index", &glyph_index)) {
			update_glyph();
		}
		if (ImGui::SliderInt("Pixel Size", &pixel_size, 8, 512)) {
			update_glyph();
		}
		if (ImGui::SliderFloat("Spread", &spread, 1.0f, 100.0f)) {
			update_glyph();
		}
		if (ImGui::DragInt2("Temp", x)) {
			update_glyph();
		}
	}
	ImGui::End();
}

void demo::destroy() {
	if (default_tex) delete default_tex;
	if (sdf_tex) delete sdf_tex;

	FT_CALL(FT_Done_Face(face));
	FT_CALL(FT_Done_FreeType(library));
}

void demo::update_glyph() {
	FT_CALL(FT_Set_Pixel_Sizes(face, pixel_size, 0));
	FT_CALL(FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER));

	FT_Bitmap sdf;
	FT_Bitmap_Init(&sdf);

	Generate_SDF(library, face->glyph, &sdf);

	//((float *)sdf.buffer)[x[1] * sdf.width + x[0]] = 1.0f;

	delete default_tex;
	delete sdf_tex;

	default_tex = new texture(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RGBA, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR);
	sdf_tex = new texture(sdf.buffer, sdf.width, sdf.rows, GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR);

	FT_Bitmap_Done( library, &sdf );
}
