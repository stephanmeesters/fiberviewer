cmake_minimum_required(VERSION 2.8.5)
 
if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
endif()

PROJECT(FiberViewer)

# Path for glew32s lib, set this yourself
set(${GLEW32S_LIB} E:/Stephan/Software/glew-1.12.0/lib/Release/Win32/glew32s.lib)

# Include source directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Include the makefiles of GLFW
add_subdirectory(./lib/glfw)
include_directories(./lib/glfw/include)

# Include GLM
include_directories(./lib/glm)

# Include TCLAP
include_directories(./lib)
link_directories(./lib)

# Find all CXX and shader files
file(GLOB CXX_FILES *.cxx)
file(GLOB SHADER_FILES ${CMAKE_SOURCE_DIR}/shaders/*)

# Add all source files to the executable
if (APPLE)
    add_executable(FiberViewer MACOSX_BUNDLE ${CXX_FILES} ${SHADER_FILES})
else()
    add_executable(FiberViewer ${CXX_FILES} ${SHADER_FILES})
endif()

# Link libraries
if (APPLE)
	target_link_libraries(FiberViewer glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES} ${PNGwriter_LIBRARIES} AntTweakBar)
else()
	target_link_libraries(FiberViewer glfw  ${GLFW_LIBRARIES} AntTweakBar 
${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW32S_LIB})
endif()

# Copy the shaders to the build dir
add_custom_command(TARGET FiberViewer PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders/ $<TARGET_FILE_DIR:FiberViewer>)
				   
# copy various dependencies
add_custom_command(TARGET FiberViewer PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/dependencies/ $<TARGET_FILE_DIR:FiberViewer>)

# Retina mode
if (APPLE)
    set_target_properties(FiberViewer PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist.in)
endif ()


























