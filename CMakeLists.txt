cmake_minimum_required(VERSION 3.5)

project(Cluster LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)     
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
aux_source_directory(./src srcs)

add_executable(Cluster
    ${srcs} 
)

target_link_libraries(Cluster PRIVATE Qt6::Widgets)