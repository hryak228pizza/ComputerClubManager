# CMakeList.txt: файл проекта CMake верхнего уровня; проведите глобальную конфигурацию
# и включите сюда подпроекты.
#
cmake_minimum_required (VERSION 3.8)

project ("ComputerClubManager")
set(CMAKE_CXX_STANDARD 17)


add_executable(task ComputerClubManager.cpp)
