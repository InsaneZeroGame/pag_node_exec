# CMakeList.txt : CMake project for pag_node_exec, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (pag_node_exec "pag_node_exec.cpp" "pag_node_exec.h" "stb_image.h" "stb_image_write.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET pag_node_exec PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(pag_node_exec ${libs})


# TODO: Add tests and install targets if needed.
