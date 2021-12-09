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

bool OpenVRTrackersModule::configure(yarp::os::ResourceFinder &rf)
{
    m_baseFrame = rf.check("tfBaseFrameName", yarp::os::Value("openVR_origin")).asString();

    yarp::os::Property tfClientCfg;
    tfClientCfg.put("device", "transformClient");
    tfClientCfg.put("local",  rf.check("tfLocal", yarp::os::Value("/OpenVRTrackers/tf")).asString());
    tfClientCfg.put("remote", rf.check("tfRemote", yarp::os::Value("/transformServer")).asString());

    if (!m_driver.open(tfClientCfg))
    {
        yError() << "Unable to open polydriver with the following options:" << tfClientCfg.toString();
        return false;
    }

    if (!m_driver.view(m_tf) || !m_tf)
    {
        yError() << "Unable to view IFrameTransform interface.";
        return false;
    }

    m_sendBuffer.resize(4,4);
    m_sendBuffer.eye();

    return true;
}

double OpenVRTrackersModule::getPeriod()
{
    return 0.01;
}

bool OpenVRTrackersModule::updateModule()
{
    std::string frameToSendName{"test"};
    m_tf->setTransform(frameToSendName, m_baseFrame, m_sendBuffer);

    return true;
}

bool OpenVRTrackersModule::close()
{
    m_driver.close();

    return true;
}
