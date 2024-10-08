cmake_minimum_required(VERSION 3.28)
project(GAME_ANIMATION VERSION 1.0.0 LANGUAGES CXX)

option(GAME_ANIMATION_HARDENING "Enable hardening options" ON)
message(STATUS "GAME_ANIMATION_HARDENING: ${GAME_ANIMATION_HARDENING}")

# Configure desired compilation options and warnings
include(cmake/compiler-warnings.cmake)
include(cmake/compiler-options.cmake)
add_library(gameanim_warnings INTERFACE)
add_library(gameanim_options INTERFACE)
setup_compiler_warnings(gameanim_warnings)
setup_compiler_options(gameanim_options)

if (GAME_ANIMATION_HARDENING)
    include(cmake/sanitizers.cmake)
    setup_sanitizers(gameanim_options)
endif ()

include(FetchContent)
SET(RNDR_BUILD_TESTS OFF)
SET(RNDR_HARDENING ${GAME_ANIMATION_HARDENING})
set(RNDR_SHARED_LIBS OFF)
FetchContent_Declare(
        rndr
        GIT_REPOSITORY https://github.com/praetorian555/rndr
        GIT_TAG master
)
FetchContent_MakeAvailable(rndr)

# Setup IMGUI ######################################################################
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG c6e0284ac58b3f205c95365478888f7b53b077e2 # release-1.89.9
)
FetchContent_MakeAvailable(imgui)
FetchContent_GetProperties(imgui SOURCE_DIR SOURCE_PATH)
set(IMGUI_SOURCE_FILES
        ${SOURCE_PATH}/imgui.cpp
        ${SOURCE_PATH}/imgui.h
        ${SOURCE_PATH}/imconfig.h
        ${SOURCE_PATH}/imgui_internal.h
        ${SOURCE_PATH}/imstb_rectpack.h
        ${SOURCE_PATH}/imstb_truetype.h
        ${SOURCE_PATH}/imstb_textedit.h
        ${SOURCE_PATH}/imgui_draw.cpp
        ${SOURCE_PATH}/imgui_demo.cpp
        ${SOURCE_PATH}/imgui_tables.cpp
        ${SOURCE_PATH}/imgui_widgets.cpp)
# Use Win32 backhand for Windows
if (MSVC)
    set(IMGUI_WINDOWS_SOURCE_FILES
            ${SOURCE_PATH}/backends/imgui_impl_win32.cpp
            ${SOURCE_PATH}/backends/imgui_impl_win32.h)
else ()
    set(IMGUI_WINDOWS_SOURCE_FILES)
endif ()
set(IMGUI_OPENGL_SOURCE_FILES
        ${SOURCE_PATH}/backends/imgui_impl_opengl3.cpp
        ${SOURCE_PATH}/backends/imgui_impl_opengl3.h)
add_library(imgui
        ${IMGUI_SOURCE_FILES}
        ${IMGUI_WINDOWS_SOURCE_FILES}
        ${IMGUI_OPENGL_SOURCE_FILES})
target_include_directories(imgui PUBLIC ${SOURCE_PATH})

# Setup Tiny GLTF ##################################################################
add_library(tinygltf STATIC third-party/tiny-gltf/src/tiny_gltf.cc third-party/tiny-gltf/include/tiny-gltf/tiny_gltf.h third-party/tiny-gltf/include/tiny-gltf/json.hpp)
target_include_directories(tinygltf PUBLIC third-party/tiny-gltf/include)
target_link_libraries(tinygltf PUBLIC gameanim_options gameanim_warnings)

set(GAME_ANIMATION_FILES
        src/types.h
        src/imgui-wrapper.cpp
        src/imgui-wrapper.h
        src/stb_image_resize2.h
        src/stb_image.h
        src/stb_image_write.h
        src/stb_image.cpp
        src/main.cpp)
add_executable(game-animation ${GAME_ANIMATION_FILES})
target_link_libraries(game-animation PRIVATE gameanim_options gameanim_warnings)
target_link_libraries(game-animation PUBLIC rndr math opal imgui tinygltf)
target_include_directories(game-animation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/shared)
target_compile_definitions(game-animation PUBLIC -DASSETS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
