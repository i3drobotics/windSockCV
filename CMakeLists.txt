cmake_minimum_required(VERSION 3.9)

# Define project
project(windsockcv VERSION 0.0.1 DESCRIPTION "Websocket for sending OpenCV images" LANGUAGES CXX)

# Use 'Release' by default for build
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Define GNU standard installation directories
include(GNUInstallDirs)

# Define source files
set(SRC_FILES 
    src/base64.cpp
    src/sha1.cpp
    src/convertimage.cpp
)

# Define server source files
set(SERVER_SRC_FILES
    ${SRC_FILES}
    src/mainserver.cpp
)

# Define server source files
set(CLIENT_SRC_FILES
    ${SRC_FILES}
    src/mainclient.cpp
)

# Define OpenCV library
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Build server application
add_executable(${PROJECT_NAME}server ${SERVER_SRC_FILES})
target_link_libraries(${PROJECT_NAME}server ${OpenCV_LIBS})

# Build client application
add_executable(${PROJECT_NAME}client ${CLIENT_SRC_FILES})
target_link_libraries(${PROJECT_NAME}client ${OpenCV_LIBS})

# Include headers in server application
target_include_directories(${PROJECT_NAME}server PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

# Include headers in client application
target_include_directories(${PROJECT_NAME}client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

if(CMAKE_BUILD_TYPE MATCHES Release)
    # Define OpenCV dlls (will be copied to install folder)
    set(OpenCV_DLLS
        ${OpenCV_DIR}/x64/vc15/bin/opencv_world3410.dll
        ${OpenCV_DIR}/x64/vc15/bin/opencv_ffmpeg3410_64.dll
    )
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    # Define OpenCV dlls (will be copied to install folder)
    set(OpenCV_DLLS
        ${OpenCV_DIR}/x64/vc15/bin/opencv_world3410d.dll
        ${OpenCV_DIR}/x64/vc15/bin/opencv_ffmpeg3410_64.dll
    )
endif()

# Install app in install directory
install(TARGETS ${PROJECT_NAME}server DESTINATION .)
# Install app in install directory
install(TARGETS ${PROJECT_NAME}client DESTINATION .)
# Install OpenCV dlls in install directory
install(FILES ${OpenCV_DLLS} DESTINATION .)