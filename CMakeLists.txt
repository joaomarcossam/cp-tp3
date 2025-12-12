cmake_minimum_required(VERSION 3.15)
project(cp-tp3
  VERSION 0.1
  LANGUAGES C
)

# C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Diretórios do projeto
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Coleta automática de todos os .c do src/
file(GLOB_RECURSE PROJECT_SOURCES
  "${PROJECT_SRC_DIR}/*.c"
)

# Dependências
find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

# Sanitizer no modo Debug (GCC → AddressSanitizer)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Debug mode: enabling AddressSanitizer")

  add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address)
endif()

add_compile_options(-fsanitize=address,leak -fno-omit-frame-pointer -g)
add_link_options(-fsanitize=address,leak)

# Executável
add_executable(main ${PROJECT_SOURCES})

# Includes do projeto
target_include_directories(main PRIVATE
  ${PROJECT_INCLUDE_DIR}
)

# MPI
if (TARGET MPI::MPI_C)
  target_link_libraries(main PRIVATE MPI::MPI_C)
else()
  # fallback (para ambientes bem antigos)
  target_include_directories(main PRIVATE ${MPI_C_INCLUDE_PATH})
  target_link_libraries(main PRIVATE ${MPI_C_LIBRARIES})
  target_compile_options(main PRIVATE ${MPI_C_COMPILE_FLAGS})
  target_link_options(main PRIVATE ${MPI_C_LINK_FLAGS})
endif()

# OpenMP
if (TARGET OpenMP::OpenMP_C)
  target_link_libraries(main PRIVATE OpenMP::OpenMP_C)
else()
  target_compile_options(main PRIVATE ${OpenMP_C_FLAGS})
  target_link_libraries(main PRIVATE ${OpenMP_C_LIBRARIES})
endif()

# Math.h
target_link_libraries(main PRIVATE m)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
