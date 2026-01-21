cmake_minimum_required(VERSION 3.5)

# Set the extra component directory to include the custom driver
set(EXTRA_COMPONENT_DIRS "../../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lsm303dlhc_test)
