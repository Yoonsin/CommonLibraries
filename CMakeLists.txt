cmake_minimum_required(VERSION 3.10)
project(CommonLibraries)

set(CMAKE_CXX_FLAGS "-D__ANDROID__ -O2 -fno-rtti -fno-exceptions -std=c++11" CACHE STRING "C++ Flags")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# Android NDK 환경인지 확인
if (ANDROID)
    message(STATUS "Building CommonLibraries for Android")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__ANDROID__")
endif()

if (ANDROID)
    include_directories(C:/android-sdk/ndk/23.2.8568313/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include)
endif()

# 서브 디렉토리 추가
add_subdirectory(Common)
add_subdirectory(IrrlichtExtensions)

# Common과 IrrlichtExtensions 라이브러리를 함께 빌드하도록 설정
add_library(CommonLibraries STATIC)
target_link_libraries(CommonLibraries CommonLibrariesCommon IrrlichtExtensions)