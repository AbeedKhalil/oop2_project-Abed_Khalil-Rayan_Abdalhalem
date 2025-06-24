cmake_minimum_required (VERSION 3.26)

project (oop2_project)
set (MY_AUTHORS "Abed_Khalil-Rayan_Abdalhalem")

include (cmake/CompilerSettings.cmake)

add_executable (${CMAKE_PROJECT_NAME})

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address>)
if (NOT MSVC)
    target_link_options(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:DEBUG>:-fsanitize=address>)
endif()

add_subdirectory (include)
add_subdirectory (src)
add_subdirectory (resources)

include (cmake/Zip.cmake)

if (WIN32)
    set (SFML_LOCATION "C:/SFML/SFML-2.6.1")
    set (SFML_DIR "${SFML_LOCATION}/lib/cmake/SFML")
endif()

find_package (SFML 2.6 COMPONENTS graphics REQUIRED)

if (WIN32)
    set_target_properties (sfml-main sfml-system sfml-window sfml-graphics sfml-audio sfml-network PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
    target_link_libraries (${CMAKE_PROJECT_NAME} sfml-graphics sfml-main)
    include (cmake/SFML.cmake)
else()
    target_link_libraries (${CMAKE_PROJECT_NAME} sfml-graphics pthread)
endif()

