cmake_minimum_required(VERSION 4.4)
project(FtwrMusicPlayer)

set(CMAKE_CXX_STANDARD 20)

# 使用 PkgConfig 查找 gtkmm
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(GST REQUIRED gstreamer-1.0) # 新增这行

add_executable(FtwrMusicPlayer src/main.cpp)

# 包含头文件目录并链接库
target_include_directories(FtwrMusicPlayer PRIVATE ${GTKMM_INCLUDE_DIRS} ${GST_INCLUDE_DIRS})
target_link_directories(FtwrMusicPlayer PRIVATE ${GTKMM_LIBRARY_DIRS} ${GST_LIBRARY_DIRS})
target_link_libraries(FtwrMusicPlayer PRIVATE ${GTKMM_LIBRARIES} ${GST_LIBRARIES})

