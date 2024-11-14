# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("pag_node_exec")

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DSWIFTSHADER)
add_definitions(-DGL_GLEXT_PROTOTYPES)
add_definitions(-DEGL_EGLEXT_PROTOTYPES)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpag/include
        src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cppFlags} -pthread")
    add_compile_options(-fPIC)
    add_definitions(-D__STDC_FORMAT_MACROS)
    SET(CMAKE_EXE_LINKER_FLAGS " -no-pie")

    find_package(Threads)
    list(APPEND libs ${CMAKE_THREAD_LIBS_INIT})
    find_package(ZLIB)
    include_directories(${ZLIB_INCLUDE_DIRS})
    list(APPEND libs ${ZLIB_LIBRARIES})
    list(APPEND libs dl)

    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libpag/linux/x64/libpag.a)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/swiftshader/linux/x64/libEGL.so)
    list(APPEND libs ${CMAKE_CURRENT_SOURCE_DIR}/vendor/swiftshader/linux/x64/libGLESv2.so)


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g -O1")
endif ()



# Include sub-projects.
add_subdirectory ("pag_node_exec")
