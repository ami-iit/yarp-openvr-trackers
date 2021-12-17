/*
 * Copyright (C) 2021 Fondazione Istituto Italiano di Tecnologia
 *
 * Licensed under either the GNU Lesser General Public License v3.0 :
 * https://www.gnu.org/licenses/lgpl-3.0.html
 * or the GNU Lesser General Public License v2.1 :
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
 * at your option.
 */

#ifndef OPENVR_TRACKERS_DRIVER_H
#define OPENVR_TRACKERS_DRIVER_H

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace openvr {
    struct Pose;
    struct TrackedDevice;
    class DevicesManager;

    enum class TrackingUniverseOrigin
    {
        Seated = 0,
        Standing = 1,
        Raw = 2,
    };

    enum class TrackedDeviceType
    {
        Invalid = 0,
        HMD = 1,
        Controller = 2,
        GenericTracker = 3,
        TrackingReference = 4,
        DisplayRedirect = 5,
    };
} // namespace openvr

struct openvr::Pose
{
    std::array<double, 3> position;
    std::array<double, 9> rotationRowMajor;
};

struct openvr::TrackedDevice
{
    size_t index;
    std::string serialNumber;
    // std::array<double, 3> position;
    // std::array<double, 9> rotationRowMajor;
    TrackedDeviceType type = TrackedDeviceType::Invalid;
};

class openvr::DevicesManager
{
public:
    DevicesManager(
        const TrackingUniverseOrigin origin = TrackingUniverseOrigin::Seated);
    ~DevicesManager();

    bool valid() const;
    bool initialize();
    bool initialized() const;

    bool addDevice(const size_t index);
    bool removeDevice(const std::string& serialNumber);
    std::vector<std::string> managedDevices() const;

    void clearEvents();
    void processEvents();

    // bool updateFromRuntime();

    // std::optional<std::string> serialNumber(const size_t index) const;
    // std::optional<std::reference_wrapper<const std::array<double, 3>>>
    // position(const std::string& serialNumber) const;
    std::optional<Pose>
    pose(const std::string& serialNumber) const;
    // std::optional<std::reference_wrapper<const std::array<double, 9>>>
    // rotationRowMajor(const std::string& serialNumber) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // OPENVR_TRACKERS_DRIVER_H
