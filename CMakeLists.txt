cmake_minimum_required(VERSION 3.19)
project(nio)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
include_directories("/home/xyzjiao/Desktop/openjdk/build/linux-x86_64-normal-server-release/jdk/include")
include_directories("/home/xyzjiao/Desktop/openjdk/build/linux-x86_64-normal-server-release/jdk/include/linux")


add_executable(nio
        main.cpp
        include/com_xyz_nio_channels_Selector.h
        include/com_xyz_nio_channels_ServerSocketChannel.h
        include/com_xyz_nio_channels_SocketChannel.h
        src/com_xyz_nio_channels_Selector.cpp
        src/com_xyz_nio_channels_ServerSocketChannel.cpp
        src/com_xyz_nio_channels_SocketChannel.cpp
        include/common.h
        include/JniTools.h
        src/JniTools.cpp src/sighandler.cpp include/sighandler.h)