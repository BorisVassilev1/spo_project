# CMakeList.txt : CMake project for pacman, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("pacman")

set(YGL_STATIC ON)
set(YGL_NO_ASSIMP ON)
set(YGL_RELATIVE_PATH "./lib/yoghurtgl")
add_subdirectory("./lib/yoghurtgl")

add_executable (pacman "main.cpp" "src/NBody.cpp")
add_definitions(-DYGL_NO_ASSIMP)
target_link_libraries(pacman PRIVATE YoghurtGL)
target_include_directories(pacman PRIVATE src)
if (MSVC)
	set_target_properties(pacman PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ../../../
	)
else (MSVC)
	set_target_properties(pacman PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ../
	)
	set_target_properties(YoghurtGL PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY ../../../
	)
endif (MSVC)
set_target_properties(msgBox PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_compile_options(pacman PRIVATE ${YGL_COMPILE_OPTIONS})
target_link_options(pacman PRIVATE ${YGL_LINK_OPTIONS})
