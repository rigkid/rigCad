file(READ "${CMAKE_CURRENT_LIST_DIR}/pack.json" PACK_JSON)
string(JSON PACK_NAME GET "${PACK_JSON}" name)

foreach(_dep IN ITEMS rigComponent rigImGui)
	if(NOT TARGET ${_dep})
		if(DEFINED RIGKIT_ROOT AND NOT RIGKIT_ROOT STREQUAL "")
			set(_dir "${RIGKIT_ROOT}/packs/${_dep}")
		else()
			set(_dir "${CMAKE_SOURCE_DIR}/packs/${_dep}")
		endif()
		if(EXISTS "${_dir}/CMakeLists.txt")
			add_subdirectory("${_dir}" "${CMAKE_BINARY_DIR}/packs/${_dep}")
		endif()
	endif()
endforeach()

foreach(_dep IN ITEMS rigCodeEditor rigPython rigManifold)
	if(NOT TARGET ${_dep})
		if(DEFINED RIGKIT_ROOT AND NOT RIGKIT_ROOT STREQUAL "")
			set(_dir "${RIGKIT_ROOT}/packs/${_dep}")
		else()
			set(_dir "${CMAKE_SOURCE_DIR}/packs/${_dep}")
		endif()
		if(EXISTS "${_dir}/CMakeLists.txt")
			add_subdirectory("${_dir}" "${CMAKE_BINARY_DIR}/packs/${_dep}")
		else()
			message(FATAL_ERROR "rigCad requires ${_dep} under packs/${_dep}")
		endif()
	endif()
endforeach()

add_library(${PACK_NAME} STATIC
	src/rigCad.cpp
	src/rigCad.h
	src/CadSketch.cpp
	src/CadSketch.h
)
target_link_libraries(${PACK_NAME} PUBLIC rigkit rigCodeEditor rigPython rigManifold)
if(TARGET rigImGui)
	target_link_libraries(${PACK_NAME} PUBLIC rigImGui)
endif()
if(TARGET rigComponent)
	target_link_libraries(${PACK_NAME} PUBLIC rigComponent)
endif()
target_include_directories(${PACK_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src>
)
message(STATUS "Configured pack: ${PACK_NAME}")
