/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#ifndef OPENVR_TRACKERS_MODULE_H
#define OPENVR_TRACKERS_MODULE_H

#include "OpenVRTrackersDriver.h"

#include <yarp/dev/IFrameTransform.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/RFModule.h>
#include <yarp/sig/Matrix.h>

#include <string>

class OpenVRTrackersModule : public yarp::os::RFModule
{
public:
    virtual bool configure(yarp::os::ResourceFinder& rf) final;

    virtual double getPeriod() final;

    virtual bool updateModule() final;

    virtual bool close() final;

private:
    yarp::dev::PolyDriver m_driver;
    yarp::dev::IFrameTransform* m_tf;
    yarp::sig::Matrix m_sendBuffer;
    std::string m_baseFrame;
    double m_period;
    openvr::DevicesManager m_manager;
};

#endif // OPENVR_TRACKERS_MODULE_H
