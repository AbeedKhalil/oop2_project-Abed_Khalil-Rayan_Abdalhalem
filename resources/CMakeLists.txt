add_subdirectory (Fonts)
add_subdirectory (Sound)
add_subdirectory (Textures)

# Copy initial high scores file to the build directory
configure_file ("highscores.txt" ${CMAKE_BINARY_DIR} COPYONLY)

