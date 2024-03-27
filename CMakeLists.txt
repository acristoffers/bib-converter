cmake_minimum_required(VERSION 3.14)

include(FetchContent)

project(bib-converter C)

find_package(PkgConfig REQUIRED)

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO REQUIRED gio-2.0)
pkg_search_module(TREESITTER REQUIRED tree-sitter)

FetchContent_Declare(
  TREESITTER_BIBER
  GIT_REPOSITORY https://github.com/acristoffers/tree-sitter-biber
  GIT_TAG master)

FetchContent_MakeAvailable(TREESITTER_BIBER)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

configure_file(${PROJECT_SOURCE_DIR}/resources/resources.xml.in
               ${PROJECT_BINARY_DIR}/resources.xml)

add_custom_command(
  OUTPUT resources.c
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND
    ${GLIB_COMPILE_RESOURCES} ARGS --generate-source
    --target=${PROJECT_BINARY_DIR}/resources.c
    ${PROJECT_BINARY_DIR}/resources.xml
  VERBATIM
  MAIN_DEPENDENCY ${PROJECT_BINARY_DIR}/resources.xml
  DEPENDS ${PROJECT_SOURCE_DIR}/resources/version)

add_custom_target(resources DEPENDS ${PROJECT_BINARY_DIR}/resources.c)

set(bib-converter-src
    ${treesitter_biber_SOURCE_DIR}/src/parser.c
    ${PROJECT_BINARY_DIR}/resources.c
    ${PROJECT_SOURCE_DIR}/src/args.c
    ${PROJECT_SOURCE_DIR}/src/bib.c
    ${PROJECT_SOURCE_DIR}/src/parse.c
    ${PROJECT_SOURCE_DIR}/src/format.c
    ${PROJECT_SOURCE_DIR}/src/main.c)

add_executable(bib-converter ${bib-converter-src})

target_include_directories(
  bib-converter PRIVATE ${PROJECT_SOURCE_DIR}/src ${TREESITTER_INCLUDE_DIRS}
                        ${GLIB_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS})

target_link_libraries(bib-converter PRIVATE ${TREESITTER_LIBRARIES}
                                            ${GLIB_LIBRARIES} ${GIO_LIBRARIES})

target_compile_options(
  bib-converter PRIVATE ${TREESITTER_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER}
                        ${GIO_CFLAGS_OTHER})

install(TARGETS bib-converter)
