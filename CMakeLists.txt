cmake_minimum_required(VERSION 3.15)

project(cp-tp3
  VERSION 0.1
  LANGUAGES C
)

# ============================================================
# Compilador e padrão da linguagem
# ============================================================
set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE STRING "C compiler" FORCE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================================
# Build type default
# ============================================================
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ============================================================
# Opções (macros)
# ============================================================
option(LOG_CODE_ENABLED "Enable logging code" OFF)
option(PARALLEL         "Enable parallel execution" ON)
option(PRINT_RESULT     "Enable result printing" ON)

# ============================================================
# Diretórios do projeto
# ============================================================
set(PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PROJECT_SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src")

# ============================================================
# Fontes
# ============================================================
file(GLOB_RECURSE PROJECT_SOURCES "${PROJECT_SRC_DIR}/*.c")

# ============================================================
# Executável
# ============================================================
add_executable(main ${PROJECT_SOURCES})

target_include_directories(main PRIVATE
  ${PROJECT_INCLUDE_DIR}
)

# ============================================================
# Macros de pré-processador
# ============================================================
target_compile_definitions(main PUBLIC
  $<$<BOOL:${LOG_CODE_ENABLED}>:LOG_CODE_ENABLED>
  $<$<BOOL:${PARALLEL}>:PARALLEL>
  $<$<BOOL:${PRINT_RESULT}>:PRINT_RESULT>
)

# ============================================================
# MPI
# ============================================================
find_package(MPI REQUIRED)
if(TARGET MPI::MPI_C)
  target_link_libraries(main PRIVATE MPI::MPI_C)
else()
  target_include_directories(main PRIVATE ${MPI_C_INCLUDE_PATH})
  target_link_libraries(main PRIVATE ${MPI_C_LIBRARIES})
  target_compile_options(main PRIVATE ${MPI_C_COMPILE_FLAGS})
  target_link_options(main PRIVATE ${MPI_C_LINK_FLAGS})
endif()

# ============================================================
# OpenMP
# ============================================================
find_package(OpenMP REQUIRED)
if(TARGET OpenMP::OpenMP_C)
  target_link_libraries(main PRIVATE OpenMP::OpenMP_C)
else()
  target_compile_options(main PRIVATE ${OpenMP_C_FLAGS})
  target_link_libraries(main PRIVATE ${OpenMP_C_LIBRARIES})
endif()
target_include_directories(main PRIVATE /usr/lib/gcc/x86_64-pc-linux-gnu/15.2.1/include)

# ============================================================
# Lib matemática
# ============================================================
target_link_libraries(main PRIVATE m)

# ============================================================
# Sanitizers (Debug)
# ============================================================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Debug mode: enabling AddressSanitizer")

  target_compile_options(main PRIVATE
    -fsanitize=address,leak
    -fno-omit-frame-pointer
    -g
  )
  
  # Link options **depois de todas as libs**
  target_link_options(main PRIVATE
    -fsanitize=address,leak
  )
endif()
