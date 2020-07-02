cmake_minimum_required(VERSION 3.3)
project(rt)

# Source directory
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

# General compilation flags
set(CXX_FLAGS_LIST
	"-Wall"
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
#	"-fsanitize=thread"
)
string(REPLACE ";" " " CXX_FLAGS_DEBUG_STR "${CXX_FLAGS_DEBUG_LIST}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS_DEBUG_STR}")

# Release compilation flags
set( CXX_FLAGS_RELEASE_LIST
	"-g"
	"-pg"
	# "-s"
	"-O3"
	"-ffast-math"
	"-ftree-vectorize"
	"-march=native"
	"-DNDEBUG"
)
string(REPLACE ";" " " CXX_FLAGS_RELEASE_STR "${CXX_FLAGS_RELEASE_LIST}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS_RELEASE_STR}")

link_libraries(
	"sfml-system"
	"sfml-window"
	"sfml-graphics"
	"assimp"
)

add_executable(
	rt
	"${PROJECT_SOURCE_DIR}/rt.cpp"
	"${PROJECT_SOURCE_DIR}/ray.cpp"
	"${PROJECT_SOURCE_DIR}/camera.cpp"
	"${PROJECT_SOURCE_DIR}/scene.cpp"
	"${PROJECT_SOURCE_DIR}/renderer.cpp"
	"${PROJECT_SOURCE_DIR}/pbr_material.cpp"
	"${PROJECT_SOURCE_DIR}/triangle_mesh.cpp"
	"${PROJECT_SOURCE_DIR}/primitive_soup.cpp"
	"${PROJECT_SOURCE_DIR}/bvh_accelerator.cpp"
)