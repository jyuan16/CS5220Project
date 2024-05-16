cmake_minimum_required(VERSION 3.14)
project(CS5220Project)
enable_language(CUDA)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

find_package(OpenMP REQUIRED)

# We require the GNU compiler for this assignment.
if (NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(Prg-Intel "PrgEnv-intel")
    set(Prg-Clang "PrgEnv-cray")
    message(WARNING
            "Must use GNU Compiler for submission. Make sure you ran:\n"
            "module swap ${Prg-${CMAKE_C_COMPILER_ID}} PrgEnv-gnu")
endif ()

add_executable(naive main.cpp naive.cpp common.h)
add_executable(openmp main.cpp openmp.cpp common.h)
add_executable(gpu main.cu gpu.cu common.h)

target_link_libraries(openmp PRIVATE OpenMP::OpenMP_CXX)

target_compile_features(gpu PRIVATE cxx_std_11)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -arch=sm_60")

# Copy the job scripts
configure_file(job-naive job-naive COPYONLY)
configure_file(job-openmp job-openmp COPYONLY)
configure_file(job-gpu job-gpu COPYONLY)
