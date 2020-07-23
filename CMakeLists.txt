cmake_minimum_required(VERSION 3.3)
project(rt)

# General compilation flags
set(CXX_FLAGS_LIST
	"-Wall"
	"-Wextra"
	"-Wno-unused-parameter"
	"--std=c++17"
	"-pthread"
)
string(REPLACE ";" " " CXX_FLAGS_STR "${CXX_FLAGS_LIST}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS_STR}")

# Debug compilation flags
set(CXX_FLAGS_DEBUG_LIST
	"-g"
	"-DDEBUG"
	"-fno-omit-frame-pointer"
	"-fsanitize=address"
	"-fsanitize=undefined"
	# "-O3"
	"-ffast-math"
	"-march=native"
#	"-fsanitize=thread"
)
string(REPLACE ";" " " CXX_FLAGS_DEBUG_STR "${CXX_FLAGS_DEBUG_LIST}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS_DEBUG_STR}")

# Release compilation flags
set(CXX_FLAGS_RELEASE_LIST
	# "-g"
	# "-pg"
	# "-s"
	"-O3"
	"-ffast-math"
	"-ftree-vectorize"
	"-march=native"
	"-DNDEBUG"
	# "-fsanitize=thread"
	# "-fsanitize=address"

)
string(REPLACE ";" " " CXX_FLAGS_RELEASE_STR "${CXX_FLAGS_RELEASE_LIST}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS_RELEASE_STR}")

add_executable(
	rt
	"${PROJECT_SOURCE_DIR}/src/rt.cpp"
	"${PROJECT_SOURCE_DIR}/src/ray.cpp"
	"${PROJECT_SOURCE_DIR}/src/camera.cpp"
	"${PROJECT_SOURCE_DIR}/src/scene.cpp"
	"${PROJECT_SOURCE_DIR}/src/renderer.cpp"
	"${PROJECT_SOURCE_DIR}/src/path_tracer.cpp"
	"${PROJECT_SOURCE_DIR}/src/primitive_collection.cpp"
	"${PROJECT_SOURCE_DIR}/src/mesh_data.cpp"
	"${PROJECT_SOURCE_DIR}/src/bvh_tree.cpp"
	"${PROJECT_SOURCE_DIR}/src/blender_jsd_loader.cpp"
	"${PROJECT_SOURCE_DIR}/src/materials/pbr_material.cpp"
	"${PROJECT_SOURCE_DIR}/src/materials/general_bsdf.cpp"
)

# Look for Assimp
find_package(ASSIMP REQUIRED)
if (ASSIMP_FOUND)
	include_directories(${ASSIMP_INCLUDE_DIRS})
	target_link_libraries(rt ${ASSIMP_LIBRARIES})
else()
	message(FATAL_ERROR "Please install Assimp!")
endif()

# Look for SFML
find_package(SFML 2 COMPONENTS system window graphics REQUIRED)
if (SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIRS})
	target_link_libraries(rt "sfml-system" "sfml-window" "sfml-graphics")
else()
	message(FATAL_ERROR "Please install SFML 2.x!")
endif()

# Look for GLM
find_package(glm REQUIRED)
if (glm_FOUND)
	include_directories(${glm_INCLUDE_DIRS})
else()
	message(FATAL_ERROR "Please install GLM!")
endif()

# Look for JSON
find_package(nlohmann_json 3.2.0 REQUIRED)
include_directories(${nlohmann_json_INCLUDE_DIRS})
target_link_libraries(rt nlohmann_json::nlohmann_json)

# Copy resources
file(COPY "${PROJECT_SOURCE_DIR}/resources" DESTINATION "${PROJECT_BINARY_DIR}")