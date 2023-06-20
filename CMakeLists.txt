cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE cmake/toolchain.cmake)

project(ProtosServerLinuxNoGui VERSION 1.0 LANGUAGES CXX)
add_definitions(-D_VER="V1.0")
add_definitions(-D_BUILD_TYPE_="${CMAKE_BUILD_TYPE}")

message("SysName: ${CMAKE_SYSTEM_NAME}")
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#    set(TARGET_USER sa)
#    set(CMAKE_INSTALL_RPATH "/home/${TARGET_USER}/${PROJECT_NAME}")
#    message(====!!!!!Check ExtPath!!!!!!====== ${CMAKE_INSTALL_RPATH})
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
#    enable_language("RC")
#    set (WIN32_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/windows/ProtosServer.rc)
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(qt_ver      Qt5)
find_package(${qt_ver}Core        REQUIRED)
find_package(${qt_ver}Widgets     REQUIRED)
find_package(${qt_ver}Network     REQUIRED)
find_package(${qt_ver}SerialPort  REQUIRED)

#add_compile_options(
#        -finput-charset=CP1251
#        -fexec-charset=CP866
#)
#set(TS_FILES ProtosServer_en_US.ts)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/App/*.*
        )

set(SOURCE_FILES
        ${SOURCES}
        #${WIN32_RESOURCES}
        )

add_subdirectory(Protos)

add_executable(${PROJECT_NAME}
        main.cpp
        ${SOURCE_FILES}
        #${WIN32_RESOURCES}
        )

target_sources(${PROJECT_NAME}
        PRIVATE
        ${SOURCE_FILES}
        )

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/App/
        )

target_link_libraries(${PROJECT_NAME}
        ${qt_ver}::Core
        ${qt_ver}::Widgets
        ${qt_ver}::Network
        ${qt_ver}::SerialPort
        Protos
        )

#target_link_options(${PROJECT_NAME}
#        PUBLIC
#        )
#target_compile_definitions(${PROJECT_NAME} PRIVATE
#        )

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Build type: Debug")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/protos_server.conf ${CMAKE_CURRENT_BINARY_DIR}/protos_server.conf COPYONLY)
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Protos/bin ${CMAKE_CURRENT_BINARY_DIR}/
    )
ENDIF()

IF(CMAKE_BUILD_TYPE MATCHES Release)
    message("Release build!")
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE true)
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Protos/bin ${CMAKE_CURRENT_BINARY_DIR}/
        )
        add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND windeployqt "$<TARGET_FILE:${PROJECT_NAME}>"
                VERBATIM
        )
    endif()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/protos_server.conf ${CMAKE_CURRENT_BINARY_DIR}/protos_server.conf COPYONLY)
    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/Makefile
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
            COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cbp

            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/Protos
    )
ENDIF()

#target_compile_options(${PROJECT_NAME}
#        PRIVATE
#        -Wall
#        -Wextra
#        -Wpedantic
##        -Werror
#        -Wduplicated-cond
#        -Wduplicated-branches
#        -Wlogical-op
#        -Wnull-dereference
#        -Wshadow
#        -Wconversion
#        -Wsign-conversion
#        )