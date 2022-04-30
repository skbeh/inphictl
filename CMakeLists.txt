cmake_minimum_required(VERSION 3.21)

project(inphictl)

add_executable(inphictl inphictl.c)

set_property(TARGET inphictl PROPERTY C_STANDARD 17)
set_property(TARGET inphictl PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
add_compile_options(-Wall -Wextra)

find_path(LIBUSB_INCLUDE_DIR
  NAMES libusb.h
  PATH_SUFFIXES "libusb-1.0")
find_library(LIBUSB_LIBRARY
  NAMES usb-1.0
  PATH_SUFFIXES "lib")

target_include_directories(inphictl PRIVATE ${LIBUSB_INCLUDE_DIR})
target_link_libraries(inphictl ${LIBUSB_LIBRARY})
