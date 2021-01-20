cmake_minimum_required(VERSION 3.15.0)
include(.cmake.conf)

project(QtDebugBridge
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Debug Bridge Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core DBus Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS)

find_package(libUsb REQUIRED)

set(CMAKE_AUTOMOC ON)

qt_build_repo_begin()

if (QT_BUILD_TESTS AND NOT QT_BUILD_STANDALONE_TESTS)
    find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Test)
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libqdb/CMakeLists.txt")
    add_subdirectory(libqdb)
endif()

if(NOT QT_BUILD_STANDALONE_TESTS)
    if (DAEMON_ONLY AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/qdbd/CMakeLists.txt")
        add_subdirectory(qdbd)
    elseif(NOT DAEMON_ONLY AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/qdb/CMakeLists.txt")
        add_subdirectory(qdb)
    endif()
endif()

if (QT_BUILD_TESTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
    if(NOT QT_BUILD_TESTS_BY_DEFAULT)
        set_property(DIRECTORY tests PROPERTY EXCLUDE_FROM_ALL TRUE)
    endif()
endif()

qt_build_repo_end()
