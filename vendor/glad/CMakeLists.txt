
# glad is used to load OpenGL functions
# https://glad.dav1d.de/

add_library(glad STATIC "src/glad.c")

target_include_directories(glad PRIVATE "include")