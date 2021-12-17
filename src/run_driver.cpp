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
#include <chrono>

int main(int /*argc*/, char** /*argv*/)
{
    std::cout << "[main] Opening the manager..." << std::endl;
    auto manager = openvr::DevicesManager();
    std::cout << "[main] ... done" << std::endl;

    std::cout << "[main] Initializing the manager..." << std::endl;
    if (!manager.initialize()) {
        std::cerr << "Failed to initialize the manager" << std::endl;
        return 1;
    }
    std::cerr << "[main] ... done" << std::endl;

    for (const auto& sn : manager.managedDevices()) {
        std::cout << "Found: " << sn << std::endl;
    }

    while (true) {

        std::this_thread::sleep_for(std::chrono::seconds(1));

        const std::string sn = "LHR-8F3A4D72";
        const auto& managedDevices = manager.managedDevices();
        if (std::find(managedDevices.begin(), managedDevices.end(), sn) == managedDevices.end())
            continue;

        const auto& poseOpt = manager.pose(sn);
        // const auto& pos = posOpt;

        if (!poseOpt.has_value()) {
            std::cerr << "Failed to get position" << std::endl;
            continue;
        }

        const auto& pose = poseOpt.value();

        // std::cerr << &pos[0] << ", " << &pos[1] << ", " << &pos[2] << std::endl;
        std::cerr << pose.position[0] << ", " << pose.position[1] << ", " << pose.position[2] << std::endl;
    }

    // std::this_thread::sleep_for(std::chrono::seconds(120));

    return 0;
}
