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
        GIT_TAG        master
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

# Setup ETC2COMP ######################################################################
FetchContent_Declare(
        etc2comp
        GIT_REPOSITORY https://github.com/google/etc2comp
        GIT_TAG 39422c1aa2f4889d636db5790af1d0be6ff3a226
)
FetchContent_Populate(etc2comp)
FetchContent_GetProperties(etc2comp SOURCE_DIR ETC2COMP_SOURCE_PATH)
file(GLOB_RECURSE ETC2COMP_SOURCES CONFIGURE_DEPENDS
        ${ETC2COMP_SOURCE_PATH}/EtcLib/Etc/*.h
        ${ETC2COMP_SOURCE_PATH}/EtcLib/EtcCodec/*.h
        ${ETC2COMP_SOURCE_PATH}/EtcLib/Etc/*.cpp
        ${ETC2COMP_SOURCE_PATH}/EtcLib/EtcCodec/*.cpp)
add_library(etc2comp ${ETC2COMP_SOURCES}
        ${ETC2COMP_SOURCE_PATH}/EtcTool/EtcFile.h
        ${ETC2COMP_SOURCE_PATH}/EtcTool/EtcFile.cpp
        ${ETC2COMP_SOURCE_PATH}/EtcTool/EtcFileHeader.h
        ${ETC2COMP_SOURCE_PATH}/EtcTool/EtcFileHeader.cpp)
target_include_directories(etc2comp PUBLIC ${ETC2COMP_SOURCE_PATH})
target_include_directories(etc2comp PUBLIC ${ETC2COMP_SOURCE_PATH}/EtcLib/Etc
        ${ETC2COMP_SOURCE_PATH}/EtcLib/EtcCodec
        ${ETC2COMP_SOURCE_PATH}/EtcTool)
target_compile_options(etc2comp PRIVATE /W1)

# Setup ASSIMP ######################################################################
include(cmake/cpm.cmake)
cpmaddpackage(
        NAME
        assimp
        GITHUB_REPOSITORY
        "assimp/assimp"
        VERSION
        5.4.2
        OPTIONS
        "ASSIMP_ASSAN OFF"
        "BUILD_SHARED_LIBS OFF"
        "ASSIMP_BUILD_ASSIMP_TOOLS OFF"
        "ASSIMP_BUILD_TESTS OFF"
        "ASSIMP_NO_EXPORT ON"
        "ASSIMP_INSTALL_PDB OFF"
        "ASSIMP_BUILD_ZLIB ON"
        "ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF"
        "ASSIMP_BUILD_OBJ_IMPORTER ON"
        "ASSIMP_BUILD_FBX_IMPORTER ON"
        "ASSIMP_BUILD_GLTF_IMPORTER ON"
        "ASSIMP_BUILD_COLLADA_IMPORTER ON"
)
SET_TARGET_PROPERTIES(assimp PROPERTIES FOLDER Extern)
SET_TARGET_PROPERTIES(zlibstatic PROPERTIES FOLDER Extern)
SET_TARGET_PROPERTIES(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER Extern)

# Setup MESHOPTIMIZER ######################################################################
cpmaddpackage(
        NAME
        meshoptimizer
        GITHUB_REPOSITORY
        "zeux/meshoptimizer"
        GIT_TAG
        "v0.20"
        OPTIONS
        "MESHOPT_WERROR ON"
)
SET_TARGET_PROPERTIES(meshoptimizer PROPERTIES FOLDER Extern)

# Setup GLI ###############################################################################
FetchContent_Declare(
        gli
        GIT_REPOSITORY https://github.com/g-truc/gli
        GIT_TAG 30808550a20ca53a255e6e1e77070493eda7b736 # 0.8.2
)
FetchContent_Populate(gli)
FetchContent_GetProperties(gli SOURCE_DIR GLI_SOURCE_PATH)
add_library(gli INTERFACE)
target_include_directories(gli INTERFACE ${GLI_SOURCE_PATH})
target_include_directories(gli INTERFACE ${GLI_SOURCE_PATH}/external/glm)
set_target_properties(gli PROPERTIES FOLDER Extern)

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
target_link_libraries(game-animation PUBLIC rndr math opal imgui etc2comp assimp meshoptimizer gli)
target_include_directories(game-animation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/shared)
target_compile_definitions(game-animation PUBLIC -DASSETS_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
