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

bool OpenVRTrackersModule::configure(yarp::os::ResourceFinder& rf)
{
    m_period = rf.check("period", yarp::os::Value(0.01)).asFloat64();
    m_baseFrame = rf.check("tfBaseFrameName", yarp::os::Value("openVR_origin"))
                      .asString();

    yarp::os::Property tfClientCfg;
    tfClientCfg.put("device", "transformClient");
    tfClientCfg.put(
        "local",
        rf.check("tfLocal", yarp::os::Value("/OpenVRTrackers/tf")).asString());
    tfClientCfg.put(
        "remote",
        rf.check("tfRemote", yarp::os::Value("/transformServer")).asString());

    if (!m_driver.open(tfClientCfg)) {
        yError() << "Unable to open polydriver with the following options:"
                 << tfClientCfg.toString();
        return false;
    }

    if (!m_driver.view(m_tf) || !m_tf) {
        yError() << "Unable to view IFrameTransform interface.";
        return false;
    }

    m_sendBuffer.resize(4, 4);
    m_sendBuffer.eye();

    if (!m_manager.initialize()) {
        yError() << "Failed to initialize the OpenVR devices manager.";
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
    // const std::string sn_tracker = "LHR-8F3A4D72"; // tracker
    
    // const std::string sn_hmd = "LHR-752422B9"; // hmd

    // const std::string sn_t1 = "LHB-7DF84991"; // tower
    // const std::string sn_t2 = "LHB-8110B10A"; // tower
    // const std::string sn_t3 = "LHB-A6772E58"; // tower

    // const auto& sn = sn_t1;

    for (const auto& sn: m_manager.managedDevices()) {
    
        if (const auto& poseOpt = m_manager.pose(sn); poseOpt.has_value()) {

            const auto& pose = poseOpt.value();

            m_sendBuffer = yarp::sig::Matrix(4, 4).eye();

            m_sendBuffer[0][0] = pose.rotationRowMajor[0];
            m_sendBuffer[0][1] = pose.rotationRowMajor[1];
            m_sendBuffer[0][2] = pose.rotationRowMajor[2];
            m_sendBuffer[1][0] = pose.rotationRowMajor[3];
            m_sendBuffer[1][1] = pose.rotationRowMajor[4];
            m_sendBuffer[1][2] = pose.rotationRowMajor[5];
            m_sendBuffer[2][0] = pose.rotationRowMajor[6];
            m_sendBuffer[2][1] = pose.rotationRowMajor[7];
            m_sendBuffer[2][2] = pose.rotationRowMajor[8];

            m_sendBuffer[0][3] = pose.position[0];
            m_sendBuffer[1][3] = pose.position[1];
            m_sendBuffer[2][3] = pose.position[2];

            m_tf->setTransform(sn, m_baseFrame, m_sendBuffer);
        }    
    }

    // std::string frameToSendName{"test"};
    // m_tf->setTransform(frameToSendName, m_baseFrame, m_sendBuffer);

    return true;
}

bool OpenVRTrackersModule::close()
{
    m_driver.close();

    return true;
}
