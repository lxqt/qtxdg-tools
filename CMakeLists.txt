cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
# CMP0000: Call the cmake_minimum_required() command at the beginning of the top-level
# CMakeLists.txt file even before calling the project() command.
# The cmake_minimum_required(VERSION) command implicitly invokes the cmake_policy(VERSION)
# command to specify that the current project code is written for the given range of CMake
# versions.
project(qtxdg-tools)

set(QTXDG_TOOLS_MAJOR_VERSION 3)
set(QTXDG_TOOLS_MINOR_VERSION 11)
set(QTXDG_TOOLS_PATCH_VERSION 0)
set(QTXDG_TOOLS_VERSION_STRING ${QTXDG_TOOLS_MAJOR_VERSION}.${QTXDG_TOOLS_MINOR_VERSION}.${QTXDG_TOOLS_PATCH_VERSION})

set(LXQTBT_MINIMUM_VERSION "0.13.0")
set(QTXDG_MINIMUM_VERSION "3.11.0")
set(QT_MINIMUM_VERSION "5.15.0")

find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)
find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Core)
find_package(Qt5Xdg ${QTXDG_MINIMUM_VERSION} REQUIRED)

include(GNUInstallDirs)             # Standard directories for installation
include(FeatureSummary)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

include(LXQtPreventInSourceBuilds)
include(LXQtCompilerSettings NO_POLICY_SCOPE)

set(CMAKE_AUTOMOC ON)

add_subdirectory(src)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/qtxdg-tools-config.cmake.in"
    "${CMAKE_BINARY_DIR}/qtxdg-tools-config.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/qtxdg-tools"
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/qtxdg-tools-config-version.cmake"
    VERSION ${QTXDG_TOOLS_VERSION_STRING}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/qtxdg-tools-config.cmake"
    "${CMAKE_BINARY_DIR}/qtxdg-tools-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/qtxdg-tools"
    COMPONENT Devel
)

install(EXPORT
    "qtxdg-tools-targets"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/qtxdg-tools"
    FILE "qtxdg-tools-targets.cmake"
    COMPONENT Devel
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
