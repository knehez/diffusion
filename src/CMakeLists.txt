set(BINARY ${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src false *.h *.cpp)

file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/src false *.h *.cpp)

add_executable(${BINARY} ${SOURCES})

target_link_libraries(${BINARY} PRIVATE  OpenCL::OpenCL)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")   
        target_compile_options(${BINARY} PRIVATE -openmp:llvm -openmp:experimental)
        target_compile_options(${BINARY} PRIVATE /fp:fast)
        target_compile_options(${BINARY} PRIVATE /arch:AVX2)
endif()    

if(CMAKE_CXX_COMPILER_ID MATCHES "CLANG")
        target_compile_options(${BINARY} PRIVATE -fp:fast)
        target_compile_options(${BINARY} PRIVATE -march:native)
endif() 

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${BINARY} PRIVATE X11 Xext)
        target_compile_options(${BINARY} PRIVATE -fopenmp)
endif()     


# extra warnings
#target_compile_options(${BINARY}_lib PRIVATE -Werror -Wall -Wextra)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/src/Release)
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/src/Debug)