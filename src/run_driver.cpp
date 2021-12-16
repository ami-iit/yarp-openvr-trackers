/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#include "OpenVRTrackersDriver.h"

#include <iostream>
#include <thread>

int main(int /*argc*/, char** /*argv*/)
{
    auto manager = openvr::DevicesManager();

    if (!manager.initialize()) {
        std::cerr << "Failed to initialize the manager" << std::endl;
        return 1;
    }

    for (const auto& sn : manager.managedDevices()) {
        std::cout << "Found: " << sn << std::endl;
    }

    return 0;
}
