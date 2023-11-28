cmake_minimum_required(VERSION 3.1)

# include(FindPkgConfig)

find_package(PkgConfig REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release
RelWithDebInfo MinSizeRel."
      FORCE)
endif()

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install" CACHE PATH
	"Choose the install path" FORCE)
endif()

# project name
project(rtrc)

# add executable
add_executable(rtrc main.cpp rtrc.cpp)

set_property(TARGET rtrc PROPERTY CXX_STANDARD 17)
set_property(TARGET rtrc PROPERTY CXX_STANDARD_REQUIRED True)

set(RTRC_DEPS_DIR ${PROJECT_SOURCE_DIR}/deps CACHE PATH "select the path which contains rtrc dependencies ")
set(RTRC_MAIN_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include CACHE PATH "select the path which contains rtrc main includes ")
set(RTRC_MAIN_EXTRA_DIR ${PROJECT_SOURCE_DIR}/rtrc_extras)

file(GLOB rtrc_deps_files "${RTRC_DEPS_DIR}/bin/*.lib" "${RTRC_DEPS_DIR}/lib/*.lib")
file(GLOB_RECURSE rtrc_header_files RELATIVE "${PROJECT_SOURCE_DIR}" "${RTRC_MAIN_INCLUDE_DIR}/*.h")
target_sources(rtrc PRIVATE ${rtrc_header_files})

foreach(_source IN ITEMS ${rtrc_header_files})
#    get_filename_component(_source_path "${_source}" PATH)
#    file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
#    string(REPLACE "/" "\\" _group_path "${_source}")
	get_filename_component(_group_path "${_source}" DIRECTORY)
	if(WIN32)
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
			string(REPLACE "/" "\\" _group_path "${_group_path}")
		endif()
	endif()
	
	source_group("${_group_path}" FILES "${_source}")
endforeach()

target_include_directories(rtrc PUBLIC "${RTRC_MAIN_INCLUDE_DIR}" "${RTRC_DEPS_DIR}/include" )
#target_link_libraries(rtrc PUBLIC ${rtrc_deps_files} )
target_link_directories(rtrc PUBLIC "${RTRC_DEPS_DIR}/bin" "${RTRC_DEPS_DIR}/lib")
target_link_libraries(rtrc PUBLIC avcodec.lib avdevice.lib avfilter.lib avformat.lib avutil.lib swresample.lib swscale.lib vulkan-1.lib)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
target_link_libraries(rtrc PUBLIC libglfw3.lib)
else()
#target_link_libraries(rtrc PUBLIC libglfw3.dll.a)
target_link_libraries(rtrc PUBLIC "${RTRC_DEPS_DIR}/lib/libglfw3.dll.a")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
add_compile_definitions(RTRC_CXX_TEMPLATE_MEMBER=)
else()
add_compile_definitions(RTRC_CXX_TEMPLATE_MEMBER=template)
endif()

add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE RTRC_CLIENT GLFW_INCLUDE_NONE NOMINMAX)

if(WIN32)
add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
endif()

install(TARGETS rtrc RUNTIME)

file(GLOB_RECURSE shared_runfiles "${RTRC_DEPS_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}" "${RTRC_DEPS_DIR}/bin/ffmpeg.exe" "${RTRC_MAIN_EXTRA_DIR}/shaders/*.spv")
install(FILES ${shared_runfiles} DESTINATION "bin")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#add_link_options("-static")
#target_link_options(rtrc PRIVATE "LINKER:-static")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
message("asd")
endif()

