cmake_minimum_required (VERSION 2.8.11)
project (CTR64)

find_package (glfw3 3.2 REQUIRED)
find_package (OpenGL REQUIRED)
find_package (Threads)

add_definitions (-D_DESKTOP)

file(GLOB ctr64_SRC
    "source/core/*.cpp"
    "source/*.cpp"
)

add_executable (ctr64 ${ctr64_SRC})
target_include_directories (ctr64 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${OPENGL_INCLUDE_DIR})
target_compile_features(ctr64 PRIVATE cxx_range_for)
target_link_libraries(ctr64 ${OPENGL_gl_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} glfw)