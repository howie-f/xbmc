# Languages and global compiler settings
set(CMAKE_CXX_STANDARD 20 CACHE STRING "CXX_STANDARD")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")
