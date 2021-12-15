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
