cmake_minimum_required(VERSION 3.2)

project(learn_imgui C CXX)

set(CMAKE_CXX_STANDARD 17)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")

# GLFW
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)

# VULKAN
find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/mainWindow.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui.cpp 
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

target_link_libraries(${PROJECT_NAME}  
    PUBLIC
        glfw
        Vulkan::Vulkan
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        ${GLFW_DIR}/include
        ${GLFW_DIR}/deps
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)


target_compile_definitions(${PROJECT_NAME}  
    PUBLIC 
        -DImTextureID=ImU64
)