include(ExternalProject)

set(OIDN_PREFIX_DIR "${CMAKE_BINARY_DIR}/OIDN")
set(OIDN_INSTALL_DIR "${OIDN_PREFIX_DIR}/install")

ExternalProject_Add(
	oidn_download
	GIT_REPOSITORY "https://github.com/OpenImageDenoise/oidn.git"
	GIT_TAG "master"
	PREFIX "${OIDN_PREFIX_DIR}"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${OIDN_INSTALL_DIR} -DTBB_STATIC_LIB=ON -DOIDN_STATIC_LIB=ON -DOIDN_APPS=OFF
	INSTALL_DIR "${OIDN_INSTALL_DIR}"
	UPDATE_COMMAND "")

set(OIDN_INCLUDE_DIR ${OIDN_INSTALL_DIR}/include)
set(OIDN_INCLUDE_DIRS ${OIDN_INCLUDE_DIR})

add_library(oidn_oidn STATIC IMPORTED)
set_target_properties(oidn_oidn PROPERTIES 
	IMPORTED_LOCATION ${OIDN_INSTALL_DIR}/lib/libOpenImageDenoise.a)
add_dependencies(oidn_oidn oidn_download)
	
add_library(oidn_common STATIC IMPORTED)
set_target_properties(oidn_common PROPERTIES 
	IMPORTED_LOCATION ${OIDN_INSTALL_DIR}/lib/libcommon.a)
add_dependencies(oidn_common oidn_download)
	
add_library(oidn_dnnl STATIC IMPORTED)
set_target_properties(oidn_dnnl PROPERTIES
	IMPORTED_LOCATION ${OIDN_INSTALL_DIR}/lib/libdnnl.a)
add_dependencies(oidn_dnnl oidn_download)
	
set(OIDN_LIBS "-Wl,--start-group" oidn_common oidn_oidn oidn_dnnl "-Wl,--end-group" m pthread tbb tbbmalloc dl stdc++)
