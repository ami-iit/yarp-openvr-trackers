find_package(Threads REQUIRED)

# ===================
# openvr-trackers-lib
# ===================

set(LIB_TARGET_NAME openvr-trackers)

set(${LIB_TARGET_NAME}_SRC
    OpenVRTrackersDriver.cpp
)

set(${LIB_TARGET_NAME}_HDR
    OpenVRTrackersDriver.h
)

add_library(
    ${LIB_TARGET_NAME}
    STATIC
    ${${LIB_TARGET_NAME}_SRC}
    ${${LIB_TARGET_NAME}_HDR})

target_include_directories(
    ${LIB_TARGET_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    ${LIB_TARGET_NAME}
    PRIVATE
    YARP::YARP_os
    Threads::Threads
    PkgConfig::openvr)

# ====================
# yarp-openvr-trackers
# ====================

set(EXE_TARGET_NAME yarp-openvr-trackers)

set(${EXE_TARGET_NAME}_SRC
    OpenVRTrackersModule.cpp
    main.cpp
)

set(${EXE_TARGET_NAME}_HDR
    OpenVRTrackersModule.h
)

add_executable(
    ${EXE_TARGET_NAME}
    ${${EXE_TARGET_NAME}_SRC}
    ${${EXE_TARGET_NAME}_HDR})

target_link_libraries(
    ${EXE_TARGET_NAME}
    PRIVATE
    YARP::YARP_os
    YARP::YARP_sig
    YARP::YARP_dev
    YARP::YARP_math
    YARP::YARP_init
    ${LIB_TARGET_NAME})

# ===============
# Install targets
# ===============

install(TARGETS ${EXE_TARGET_NAME} DESTINATION bin)
