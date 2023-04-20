cmake_minimum_required(VERSION 3.22)
project(liquidfun_evo_sim)

set(CMAKE_CXX_STANDARD 14)
set(LIQUIDFUN_INCLUDE_DIR "C:/Users/rwill/CLionProjects/liquidfun/liquidfun/Box2D/Box2D")
set(LIQUIDFUN_LIBRARY "C:/Users/rwill/CLionProjects/liquidfun/liquidfun/Box2D/Box2D/Debug")


link_directories("C:/Users/rwill/CLionProjects/liquidfun/liquidfun/Box2D")
include_directories("C:/Users/rwill/CLionProjects/liquidfun/liquidfun/Box2D")
include_directories("C:/Users/rwill/Downloads/glfw-3.3.8.bin.WIN64/glfw-3.3.8.bin.WIN64/include")
include_directories("C:/Users/rwill/Downloads/glew-2.1.0-win32/glew-2.1.0/include")

find_package(OpenGL REQUIRED)


add_executable(liquidfun_evo_sim
        src/main.cpp
        src/rendering.cpp
        src/rendering.h
        src/creature.cpp
        src/creature.h
        )

target_link_libraries(liquidfun_evo_sim PRIVATE OpenGL::GL OpenGL::GLU
        "C:/Users/rwill/CLionProjects/liquidfun/liquidfun/Box2D/Box2D/Debug/liquidfun.lib"
        "C:/Users/rwill/Downloads/glfw-3.3.8.bin.WIN64/glfw-3.3.8.bin.WIN64/lib-vc2022/glfw3dll.lib"
        "C:/Users/rwill/Downloads/glew-2.1.0-win32/glew-2.1.0/lib/Release/x64/glew32.lib"
        )
