cmake_minimum_required(VERSION 3.5)
project (time_support)

SET(ADD_CHRONO_TIME "-DCHRONO_TIME")

add_subdirectory (src)
add_subdirectory (src/unitTests)
