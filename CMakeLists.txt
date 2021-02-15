cmake_minimum_required(VERSION 3.12)
project(tp-size)

find_package(ROOT)

root_generate_dictionary(TPClassesDict TPClasses.h LINKDEF LinkDef.h)

add_library(tpclasses SHARED TPClasses.cpp TPClasses.h TPClassesDict.cxx)
target_include_directories(tpclasses PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(tpclasses PUBLIC ROOT::Core)

add_executable(tp-size tp-size.cpp)
target_link_libraries(tp-size tpclasses ROOT::Core ROOT::Tree)
