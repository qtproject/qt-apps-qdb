if (MSVC)
   set(WINDOWS_LIBUSB_PATH "$ENV{LIBUSB_PATH}/x64-windows-static")
   if (TEST_architecture_arch STREQUAL arm64)
      set(WINDOWS_LIBUSB_PATH "$ENV{LIBUSB_PATH}/arm64-windows-static")
   endif()
endif()

if (MACOS)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif()

find_library (LIBUSB_LIBRARY
   NAMES libusb libusb-1.0 usb-1.0
   PATHS "/usr/lib" "/usr/local/lib" "${WINDOWS_LIBUSB_PATH}/lib")

find_path (LIBUSB_INCLUDEDIR
   NAMES libusb.h libusb-1.0.h
   PATHS "/usr/local/include" "${WINDOWS_LIBUSB_PATH}/include"
   PATH_SUFFIXES "include" "libusb" "libusb-1.0")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libUsb DEFAULT_MSG
                                  LIBUSB_LIBRARY
                                  LIBUSB_INCLUDEDIR)

if (LIBUSB_FOUND AND NOT TARGET libUsb::libUsb)
   add_library(libUsb::libUsb STATIC IMPORTED)
   set_target_properties(
      libUsb::libUsb
      PROPERTIES
         INTERFACE_INCLUDE_DIRECTORIES "${LIBUSB_INCLUDEDIR}"
         IMPORTED_LOCATION "${LIBUSB_LIBRARY}")
   if (MSVC OR MINGW)
      set_target_properties(
         libUsb::libUsb
         PROPERTIES
            IMPORTED_IMPLIB "${LIBUSB_LIBRARY}"
      )
   endif()
endif()
