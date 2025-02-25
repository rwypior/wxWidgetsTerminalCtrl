cmake_minimum_required (VERSION 3.26)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_definitions(-DUNICODE -D_UNICODE)

project ("wxterminal")
add_library(wxterminal)
add_library(wxterminal::wxterminal ALIAS wxterminal)

# Add source to this project's executable.
target_sources(wxterminal PRIVATE "include/wxterminal/terminalCtrl.h" "src/terminalCtrl.cpp")

set_property(TARGET wxterminal PROPERTY CXX_STANDARD 17)

target_include_directories (wxterminal PUBLIC 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)
target_link_libraries(wxterminal PUBLIC wx::core wx::base)

# Install

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ConfigPackageLocation lib/cmake/wxterminal)

configure_package_config_file(
        cmake/wxterminalConfig.cmake.in
	    "${CMAKE_CURRENT_BINARY_DIR}/wxterminal/wxterminalConfig.cmake"
	    INSTALL_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/wxterminal"
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/wxterminal/wxterminalConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY AnyNewerVersion
)

install(
        TARGETS wxterminal EXPORT wxterminalTargets
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
        DIRECTORY "include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(EXPORT wxterminalTargets
        FILE wxterminalTargets.cmake
        NAMESPACE wxterminal::
        DESTINATION ${ConfigPackageLocation}
)

install(FILES 
            "${CMAKE_CURRENT_BINARY_DIR}/wxterminal/wxterminalConfig.cmake" 
            "${CMAKE_CURRENT_BINARY_DIR}/wxterminal/wxterminalConfigVersion.cmake"
        DESTINATION ${ConfigPackageLocation}
)

export(PACKAGE wxterminal)