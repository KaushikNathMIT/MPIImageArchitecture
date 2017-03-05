cmake_minimum_required(VERSION 3.6)
project(MPIImageArchitecture)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)

find_package(MPI REQUIRED)
add_executable(MPIImageArchitecture ${SOURCE_FILES})
include_directories(${MPI_INCLUDE_PATH})
if(MPI_COMPILE_FLAGS)
    set_target_properties(MPIImageArchitecture PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(MPIImageArchitecture PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()

target_link_libraries(MPIImageArchitecture ${MPI_LIBRARIES})
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( MPIImageArchitecture ${OpenCV_LIBS} )