project (GLTemplate)

cmake_minimum_required (VERSION 2.8)

set(GLEW_SOURCE "${PROJECT_SOURCE_DIR}/library/glew-1.10.0/src/glew.c")
add_subdirectory (library/glfw-3.0.3)
add_subdirectory (library/glm)

include_directories ( ${GLEW_INCLUDE_DIR} )
include_directories ( ${GLFW_INCLUDE_DIR} )
include_directories ( ${GLM_INCLUDE_DIR} )

include_directories ("${PROJECT_SOURCE_DIR}/library")
include_directories ("${PROJECT_SOURCE_DIR}/library/glew-1.10.0/include")
include_directories ("${PROJECT_SOURCE_DIR}/library/glfw-3.0.3/include")

add_library( glew STATIC
	${GLEW_SOURCE}
)

file(GLOB SRC_FILES

    "src/*.cpp"
    "src/load/*.cpp"
    "src/buffer/*.cpp"
    "src/components/*.cpp"
    "src/components/*.c"
    "src/control/*.cpp"
    "src/shader/*.cpp"
    "src/pipeline/*.cpp"
)

# copy shaders to binary folder
file( COPY "${PROJECT_SOURCE_DIR}/glsl/" DESTINATION "${PROJECT_BINARY_DIR}/glsl/")

add_definitions(-std=c++11)
add_executable (GLMain ${SRC_FILES})
target_link_libraries (GLMain glew glfw ${GLFW_LIBRARIES})
