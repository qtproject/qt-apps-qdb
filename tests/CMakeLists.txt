find_package(Qt6 COMPONENTS Test REQUIRED)

add_subdirectory(qdbmessagetest)
add_subdirectory(stream)
add_subdirectory(subnet)
add_subdirectory(servicetest)
add_subdirectory(streamtest)

qt_build_tests()
