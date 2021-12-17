/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#include "OpenVRTrackersModule.h"
#include <yarp/os/LogStream.h>

namespace openvr_trackers_module {
    constexpr double DefaultPeriod = 0.010;
    const std::string DefaultTfLocal = "/OpenVRTrackers/tf";
    const std::string DefaultTfRemote = "/transformServer";
    const std::string DefaultTfBaseFrameName = "openVR_origin";
    const std::string ModuleName = "OpenVRTrackersModule";
    const std::string LogPrefix = ModuleName + ":";
} // namespace openvr_trackers_module

bool OpenVRTrackersModule::configure(yarp::os::ResourceFinder& rf)
{
    // ===========================
    // Check configuration options
    // ===========================

    // Try to find the "period" entry
    if (!(rf.check("period") && rf.find("period").isFloat64())) {
        yInfo() << openvr_trackers_module::LogPrefix << "Using default period:"
                << openvr_trackers_module::DefaultPeriod << "s";
        m_period = openvr_trackers_module::DefaultPeriod;
    }
    else {
        m_period = rf.find("period").asFloat64();
    }

    // Try to find the "tfBaseFrameName" entry
    if (!(rf.check("tfBaseFrameName")
          && rf.find("tfBaseFrameName").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix
                << "Using default tfBaseFrameName:"
                << openvr_trackers_module::DefaultTfBaseFrameName;
        m_baseFrame = openvr_trackers_module::DefaultTfBaseFrameName;
    }
    else {
        m_baseFrame = rf.find("tfBaseFrameName").asString();
    }

    // Try to find the "tfLocal" entry
    std::string tfLocal;
    if (!(rf.check("tfLocal") && rf.find("tfLocal").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix << "Using default tfLocal:"
                << openvr_trackers_module::DefaultTfLocal;
        tfLocal = openvr_trackers_module::DefaultTfLocal;
    }
    else {
        tfLocal = rf.find("tfLocal").asString();
    }

    // Try to find the "tfRemote" entry
    std::string tfRemote;
    if (!(rf.check("tfRemote") && rf.find("tfRemote").isString())) {
        yInfo() << openvr_trackers_module::LogPrefix
                << "Using default tfRemote:"
                << openvr_trackers_module::DefaultTfRemote;
        tfRemote = openvr_trackers_module::DefaultTfRemote;
    }
    else {
        tfRemote = rf.find("tfRemote").asString();
    }

    // Create configuration of the "transformClient" device
    yarp::os::Property tfClientCfg;
    tfClientCfg.put("device", "transformClient");
    tfClientCfg.put("local", tfLocal);
    tfClientCfg.put("remote", tfRemote);

    // Open the transformClient device
    if (!m_driver.open(tfClientCfg)) {
        yError() << openvr_trackers_module::LogPrefix
                 << "Unable to open polydriver with the following options:"
                 << tfClientCfg.toString();
        return false;
    }

    // Extract and store the IFrameTransform interface
    if (!(m_driver.view(m_tf) && m_tf)) {
        yError() << openvr_trackers_module::LogPrefix
                 << "Unable to view IFrameTransform interface.";
        return false;
    }

    // Initialize the transform buffer
    m_sendBuffer.resize(4, 4);
    m_sendBuffer.eye();

    // Initialize the OpenVR driver
    if (!m_manager.initialize()) {
        yError() << openvr_trackers_module::LogPrefix
                 << "Failed to initialize the OpenVR devices manager.";
        return false;
    }

    return true;
}

double OpenVRTrackersModule::getPeriod()
{
    return m_period;
}

bool OpenVRTrackersModule::updateModule()
{
    // Iterate over all the managed devices of the driver
    for (const auto& sn : m_manager.managedDevices()) {

        if (const auto& poseOpt = m_manager.pose(sn); poseOpt.has_value()) {

            // Extract the pose of the device
            const openvr::Pose& pose = poseOpt.value();

            // Compute the prefix of the transform based on the device type.
            // The final name will be "{tf_name_prefix}/{serial_number}".
            const std::string tfNamePrefix = [&]() {
                std::string prefix;

                switch (m_manager.type(sn)) {
                    case openvr::TrackedDeviceType::HMD:
                        prefix = "/hmd/";
                        break;
                    case openvr::TrackedDeviceType::Controller:
                        prefix = "/controllers/";
                        break;
                    case openvr::TrackedDeviceType::GenericTracker:
                        prefix = "/trackers/";
                        break;
                    default:
                        break;
                }
                return prefix;
            }();

            // Reset the transform
            m_sendBuffer.eye();

            // Fill the rotation of the transform using the row-major
            // serialization used by the driver
            m_sendBuffer[0][0] = pose.rotationRowMajor[0];
            m_sendBuffer[0][1] = pose.rotationRowMajor[1];
            m_sendBuffer[0][2] = pose.rotationRowMajor[2];
            m_sendBuffer[1][0] = pose.rotationRowMajor[3];
            m_sendBuffer[1][1] = pose.rotationRowMajor[4];
            m_sendBuffer[1][2] = pose.rotationRowMajor[5];
            m_sendBuffer[2][0] = pose.rotationRowMajor[6];
            m_sendBuffer[2][1] = pose.rotationRowMajor[7];
            m_sendBuffer[2][2] = pose.rotationRowMajor[8];

            // Fill the position of the transform
            m_sendBuffer[0][3] = pose.position[0];
            m_sendBuffer[1][3] = pose.position[1];
            m_sendBuffer[2][3] = pose.position[2];

            // Publish the transform
            m_tf->setTransform(tfNamePrefix + sn, m_baseFrame, m_sendBuffer);
        }
    }

    return true;
}

bool OpenVRTrackersModule::close()
{
    m_driver.close();
    return true;
}
