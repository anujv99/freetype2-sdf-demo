# same as glfw
cmake_minimum_required(VERSION 3.1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)

# demo project
project(freetype-sdf-demo)

# compile vendor CMakeLists.txt
# glfw				- for window and context creation
# glad				- for loading opengl functions
# imgui				- for creating gui
# freetype2-sdf			- standalone implementation for generating sdf from outline
# glm				- header only math library
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/freetype2-sdf)

# simply include all files in ./src folder
file(GLOB_RECURSE demo_source_files "./src/*.*")
add_executable(freetype-sdf-demo ${demo_source_files})

# add external libraries include directories to freetype-sdf-demo
target_include_directories(freetype-sdf-demo PRIVATE 
	"vendor/glfw/include"
	"vendor/glad/include"
	"vendor/freetype2-sdf/freetype/include"
	"vendor/freetype2-sdf/src/"
	"vendor/imgui"
	"vendor/glm"
)

# link external libraries to freetype-sdf-demo
target_link_libraries(freetype-sdf-demo PRIVATE glfw glad imgui freetype2-sdf)

# find and link opengl library
find_package(OpenGL)
target_link_libraries(freetype-sdf-demo PRIVATE ${OPENGL_gl_LIBRARY})

# if compiler is MSVC set startup project
if (MSVC)
	set(VS_STARTUP_PROJECT freetype-sdf-demo)
endif()