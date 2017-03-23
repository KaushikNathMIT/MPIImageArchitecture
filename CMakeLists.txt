cmake_minimum_required(VERSION 3.6)
project(MPIImageArchitecture)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES SampleWorking.cpp StaticImageProcessing.cpp StaticImageProcessing.h VideoProcessing.cpp VideoProcessing.h)

find_package(MPI REQUIRED)
add_executable(MPIImageArchitectureSample SampleWorking.cpp)
add_executable(MPIImageArchitectureStatic StaticImageProcessing.cpp StaticImageProcessing.h)
add_executable(MPIImageArchitectureVideo VideoProcessing.cpp VideoProcessing.h)
include_directories(${MPI_INCLUDE_PATH})
if(MPI_COMPILE_FLAGS)
    set_target_properties(MPIImageArchitectureSample PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
    set_target_properties(MPIImageArchitectureStatic PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
    set_target_properties(MPIImageArchitectureVideo PROPERTIES
            COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
    set_target_properties(MPIImageArchitectureSample PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
    set_target_properties(MPIImageArchitectureStatic PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")
    set_target_properties(MPIImageArchitectureVideo PROPERTIES
            LINK_FLAGS "${MPI_LINK_FLAGS}")

endif()

target_link_libraries(MPIImageArchitectureSample ${MPI_LIBRARIES})
target_link_libraries(MPIImageArchitectureStatic ${MPI_LIBRARIES})
target_link_libraries(MPIImageArchitectureVideo ${MPI_LIBRARIES})
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries( MPIImageArchitectureSample ${OpenCV_LIBS} )
target_link_libraries( MPIImageArchitectureStatic ${OpenCV_LIBS} )
target_link_libraries( MPIImageArchitectureVideo ${OpenCV_LIBS} )