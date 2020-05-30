
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <imgui.h>

// include helper files to easily initialize and render imgui
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_glfw.cpp>
#include <examples/imgui_impl_opengl3.h>
#include <examples/imgui_impl_opengl3.cpp>

#include "error.h"
#include "openglmanager.h"
#include "demo.h"

#define GLFW_ERR()\
	const char * error = NULL;\
	glfwGetError(&error)

int main(int argc, char * argv[]) {

	GLFWwindow * window				= NULL;
	const char WINDOW_TITLE[]		= "freetype signed distnace field demo";
	const uint32_t WINDOW_WIDTH		= 1280u;
	const uint32_t WINDOW_HEIGHT	= 720u;

	// first initialize glfw
	if (glfwInit() != GLFW_TRUE) {
		GLFW_ERR();
		LOG_ERROR("failed to initialize glfw: %s", error);
	}

	// set opengl version (3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	// use core opengl profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
							  NULL, NULL);
	if (window == NULL) {
		GLFW_ERR();
		LOG_ERROR("failed to create glfw window: %s", error);
		glfwTerminate();
		return -1;
	}

	// create opengl context
	glfwMakeContextCurrent(window);

	// load opengl functions
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("failed to load OpenGL functions");
		return -1;
	}

	// check for opengl version
	GLint versionMajor, versionMinor;
	GL_CALL(glGetIntegerv(GL_MAJOR_VERSION, &versionMajor));
	GL_CALL(glGetIntegerv(GL_MINOR_VERSION, &versionMinor));

	// version must be greater than 3.3 otherwise several features won't work (e.g. glTexImage2DMultisample)
	if (versionMajor <= 3 && versionMinor < 3) {
		LOG_ERROR("opengl version must be 3.3 (got : %d.%d)", versionMajor, versionMinor);
		return -1;
	}

	// initialize openglmanager
	if (!opengl_manager::init(500, 500)) {
		return -1;
	}

	// setup imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO(); (void)io;

	ImGuiStyle & style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;

	ImGui::StyleColorsDark();

	const char GLSL_VERSION[] = "#version 330 core";

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(GLSL_VERSION);

	// initialize demo 
	demo::init();

	// disable v-sync
	glfwSwapInterval(0);

	// main loop
	while (!glfwWindowShouldClose(window)) {
		glfwWaitEvents();

		// clear the screen for drawing
		GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

		// update and draw the demo
		demo::update();

		// begin imgui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// show the guis
		opengl_manager::gui();
		demo::gui();

		// render imgui data
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// shutdown imgui
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// destroy the demo
	demo::destroy();

	// destroy openglmanager
	opengl_manager::destroy();

	// destroy window
	glfwDestroyWindow(window);

	// quit glfw
	glfwTerminate();
	return 0;
}
