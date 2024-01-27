# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.22)

# Set the project name
project (cliz)

# find a boost install with the libraries system
find_package(nlohmann_json REQUIRED)
find_package(cppzmq REQUIRED)
find_package(argh REQUIRED)
find_package(cppcodec REQUIRED)

# Add an executable
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${nlohmann_json_LIBRARIES}
        ${cppzmq_LIBRARIES}
        ${argh_LIBRARIES}
        ${cppcodec_LIBRARIES}
)
