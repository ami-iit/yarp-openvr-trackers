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

#include <openvr.h>
#include <yarp/os/LogStream.h>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>

// ==============
// DevicesManager
// ==============

class openvr::DevicesManager::Impl
{
public:
    using TrackedDeviceSerialNumber = std::string;
    std::unordered_map<TrackedDeviceSerialNumber, TrackedDevice> devices;

    vr::IVRSystem* vr = nullptr;
    TrackingUniverseOrigin origin;

    std::thread detector;

    mutable std::recursive_mutex mutex;

    static bool DeviceTypeIsSupported(const TrackedDeviceType type) {

        switch (type)
        {
            case TrackedDeviceType::HMD:
            case TrackedDeviceType::Controller:
            case TrackedDeviceType::GenericTracker:
                return true;
            default:
                return false;
        }
    }
};

openvr::DevicesManager::DevicesManager(const TrackingUniverseOrigin origin)
    : pImpl{std::make_unique<Impl>()}
{
    pImpl->origin = origin;
}

openvr::DevicesManager::~DevicesManager()
{
    if (this->initialized()) {
        // Tear down the runtime
        pImpl->vr = nullptr;
        vr::VR_Shutdown();

        // Wait the processor thread to terminate
        pImpl->detector.join();
    }
}

bool openvr::DevicesManager::initialized() const
{
    const auto lock = std::unique_lock(pImpl->mutex);
    return pImpl->vr && !std::string(pImpl->vr->GetRuntimeVersion()).empty();
}

bool openvr::DevicesManager::initialize()
{
    if (this->initialized()) {
        yError() << "Already initialized";
        return false;
    }

    const auto lock = std::unique_lock(pImpl->mutex);

    // =================================
    // Detect and track existing devices
    // =================================

    yDebug() << "Initializing OpenVR DeviceManager";
    vr::EVRInitError eError = vr::VRInitError_None;

    // Start the application in Background:
    //
    // The application will not start SteamVR.
    // If it is not already running the call with VR_Init will fail
    // with VRInitError_Init_NoServerForBackgroundApp.
    //
    // https://github.com/ValveSoftware/openvr/wiki/API-Documentation#initialization-and-cleanup
    //
    if (pImpl->vr = vr::VR_Init(&eError, vr::VRApplication_Background);
        !pImpl->vr) {
        pImpl->vr = nullptr;
        yError() << "Failed to initialize VR runtime";
        yError() << vr::VR_GetVRInitErrorAsEnglishDescription(eError);
        return false;
    }

    yDebug() << "OpenVR runtime correctly started";
    yDebug() << "Scanning for existing devices";

    const auto connectedDevicesIndices = [&]() -> std::vector<size_t> {
        std::vector<size_t> indices = {};

        for (size_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
            if (pImpl->vr->IsTrackedDeviceConnected(i)) {
                indices.push_back(i);
            }
        }

        return indices;
    }();

    yDebug() << "Found" << connectedDevicesIndices.size() << "devices";

    for (const auto deviceIndex : connectedDevicesIndices) {
        yDebug() << "Inserting device with index" << deviceIndex;

        if (!this->addDevice(deviceIndex)) {
            yError() << "Failed to add device with index" << deviceIndex;
            return false;
        }
    }

    // ==================================
    // Execute a thread to process events
    // ==================================

    // Create the detector thread
    auto detectorLoop = [this]() {
        yDebug() << "Detector thread: starting";

        // this->clearEvents();

        while (this->initialized()) {
            this->processEvents();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        yDebug() << "Detector thread: exiting";
    };

    // Start the thread waiting initialization
    pImpl->detector = std::thread(detectorLoop);

    yInfo("VR runtime succesfully initialized");
    return true;
}

bool openvr::DevicesManager::addDevice(const size_t index)
{
    const auto lock = std::unique_lock(pImpl->mutex);

    // Make sure the device is connected
    yDebug() << "Checking if device is connected";
    if (!pImpl->vr->IsTrackedDeviceConnected(index)) {
        yError() << "Failed to add unconnected device with index" << index;
        return false;
    }

    // Get the Serial Number of the device
    char buffer[vr::k_unMaxPropertyStringSize];
    const uint32_t numBytes = pImpl->vr->GetStringTrackedDeviceProperty(
        index,
        vr::Prop_SerialNumber_String,
        buffer,
        vr::k_unMaxPropertyStringSize);
    const std::string serialNumber(buffer);

    // Get the type of the device
    const TrackedDeviceType type = TrackedDeviceType(
        pImpl->vr->GetTrackedDeviceClass(index));

    if (!Impl::DeviceTypeIsSupported(type)) {
        yInfo() << "The device" << serialNumber << "has unsupported type";
        return true;
    }

    // Create the device
    yDebug() << "Adding device" << serialNumber << " (index =" << index << ", type =" << int(type) << ")";
    const auto device = [&]() {
        TrackedDevice device;
        device.type = type;
        device.index = index;
        device.serialNumber = serialNumber;
        return device;
    }();

    // Make sure the device is not already there
    if (pImpl->devices.find(device.serialNumber) != pImpl->devices.end()) {
        yError() << "Failed to insert device" << device.serialNumber
                 << ". It was already inserted previously.";
        return false;
    }

    // Insert the new device
    pImpl->devices.insert(std::make_pair(device.serialNumber, device));
    yInfo() << "Device " << device.serialNumber << "inserted (index=" << index
            << ")";
    return true;
}

bool openvr::DevicesManager::removeDevice(const std::string& serialNumber)
{
    const auto lock = std::unique_lock(pImpl->mutex);
    const auto& device = pImpl->devices.extract(serialNumber);

    if (device.empty()) {
        yError() << "Device with serial" << serialNumber << "not found";
        return false;
    }

    yDebug() << "Removing device with serial" << serialNumber;
    return true;
}

std::vector<std::string> openvr::DevicesManager::managedDevices() const
{
    std::vector<std::string> managedDevicesSerials;
    managedDevicesSerials.reserve(pImpl->devices.size());

    for (const auto& [serial, _] : pImpl->devices) {
        managedDevicesSerials.push_back(serial);
    }

    return managedDevicesSerials;
}

void openvr::DevicesManager::clearEvents()
{
    size_t number = 0;
    vr::VREvent_t event;
    const auto lock = std::unique_lock(pImpl->mutex);

    while (pImpl->vr->PollNextEvent(&event, sizeof(event))) {
        number++;
    }

    yDebug() << "Cleared" << number << "events";
}

void openvr::DevicesManager::processEvents()
{
    vr::VREvent_t event;
    const auto lock = std::unique_lock(pImpl->mutex);

    while (pImpl->vr->PollNextEvent(&event, sizeof(event))) {

        yDebug() << "Received event:"
                 << pImpl->vr->GetEventTypeNameFromEnum(
                        vr::EVREventType(event.eventType));

        yWarning() << event.trackedDeviceIndex;

        switch (event.eventType) {
            case vr::VREvent_TrackedDeviceActivated:
                this->addDevice(event.trackedDeviceIndex);
                break;
            case vr::VREvent_TrackedDeviceDeactivated:
                for (const auto& [sn, device]: pImpl->devices) {
                    if (device.index == event.trackedDeviceIndex)
                        this->removeDevice(sn);
                }
                break;
            case vr::VREvent_TrackedDeviceUpdated:
                // index no
                break;
            case vr::VREvent_TrackedDeviceRoleChanged:
                break;
            case vr::VREvent_TrackedDeviceUserInteractionStarted:
                break;
            case vr::VREvent_TrackedDeviceUserInteractionEnded:
                break;
            case vr::VREvent_Quit:
                pImpl->vr->AcknowledgeQuit_Exiting();

                for (const auto& serial : this->managedDevices()) {
                    if (!this->removeDevice(serial)) {
                        yWarning()
                            << "Failed to remove device with serial" << serial;
                    }
                }

                pImpl->vr = nullptr;
                vr::VR_Shutdown();
                break;
            default:
                break;
        }

        if (!pImpl->vr) {
            break;
        }
    }
}

// bool openvr::DevicesManager::updateFromRuntime()
// {
//     if (!this->initialized()) {
//         yError() << "Failed to read data from the runtime, the manager is "
//                  << "not initialized";
//         return false;
//     }

//     const auto lock = std::unique_lock(pImpl->mutex);

//     return false;
// }

// std::optional<std::reference_wrapper<const std::array<double, 3>>>
// std::array<double, 3>
std::optional<openvr::Pose>
openvr::DevicesManager::pose(const std::string& serialNumber) const
{
    if (!this->initialized()) {
        yError() << "Failed to read data from the runtime, the manager is "
                 << "not initialized";
        // return {};
        return std::nullopt;
    }

    const auto lock = std::unique_lock(pImpl->mutex);

    if (pImpl->devices.find(serialNumber) == pImpl->devices.end()) {
        yError();
        // return {};
        return std::nullopt;
    }

    if (!pImpl->vr->IsTrackedDeviceConnected(
            pImpl->devices[serialNumber].index)) {
        yError();
        // return {};
        return std::nullopt;
    }

    vr::TrackedDevicePose_t pose;
    vr::VRControllerState_t state;

    pImpl->vr->GetControllerStateWithPose(
        vr::ETrackingUniverseOrigin(pImpl->origin),
        pImpl->devices[serialNumber].index,
        &state,
        sizeof(state),
        &pose);

    if (!pose.bPoseIsValid) {
        yWarning() << "The pose of device" << serialNumber << "is not valid";
        // return {};
        return std::nullopt;
    }

    if (pose.eTrackingResult
        != vr::ETrackingResult::TrackingResult_Running_OK) {
        yError() << "The state of the output of device" << serialNumber
                 << "is not ok";
        return std::nullopt;
        // return {};
    }

    // pImpl->devices[serialNumber].position = {
        // pose.mDeviceToAbsoluteTracking.m[0][3],
        // pose.mDeviceToAbsoluteTracking.m[1][3],
        // pose.mDeviceToAbsoluteTracking.m[2][3]};


    Pose out;
    out.position = {
        pose.mDeviceToAbsoluteTracking.m[0][3],
        pose.mDeviceToAbsoluteTracking.m[1][3],
        pose.mDeviceToAbsoluteTracking.m[2][3],
        };
    out.rotationRowMajor = {
        pose.mDeviceToAbsoluteTracking.m[0][0], pose.mDeviceToAbsoluteTracking.m[0][1], pose.mDeviceToAbsoluteTracking.m[0][2],
        pose.mDeviceToAbsoluteTracking.m[1][0], pose.mDeviceToAbsoluteTracking.m[1][1], pose.mDeviceToAbsoluteTracking.m[1][2],
        pose.mDeviceToAbsoluteTracking.m[2][0], pose.mDeviceToAbsoluteTracking.m[2][1], pose.mDeviceToAbsoluteTracking.m[2][2],
    };
    return out;

    // return pImpl->devices[serialNumber].position;
    // return std::ref(pImpl->devices[serialNumber].position);
    // return {
        // pose.mDeviceToAbsoluteTracking.m[0][3],
        // pose.mDeviceToAbsoluteTracking.m[1][3],
        // pose.mDeviceToAbsoluteTracking.m[2][3]};
}
