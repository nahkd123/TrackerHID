# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c hiddev.c imu_bmi160.c
    INCLUDE_DIRS "."
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES tinyusb driver
    PRIV_REQUIRES       # optional, list the private requirements
)
