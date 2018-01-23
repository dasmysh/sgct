/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#include <sgct/Engine.h>

#include "../include/vrpn/vrpn_Tracker.h"
#include "../include/vrpn/vrpn_Button.h"
#include "../include/vrpn/vrpn_Analog.h"

#include <sgct/SGCTTracker.h>
#include <sgct/ClusterManager.h>
#include <sgct/MessageHandler.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

struct VRPNPointer
{
    vrpn_Tracker_Remote * mSensorDevice;
    vrpn_Analog_Remote * mAnalogDevice;
    vrpn_Button_Remote * mButtonDevice;
};

struct VRPNTracker
{
    std::vector<VRPNPointer> mDevices;
};

std::vector<VRPNTracker> gTrackers;

void VRPN_CALLBACK update_tracker_cb(void *userdata, const vrpn_TRACKERCB t );
void VRPN_CALLBACK update_button_cb(void *userdata, const vrpn_BUTTONCB b );
void VRPN_CALLBACK update_analog_cb(void * userdata, const vrpn_ANALOGCB a );

void samplingLoop(void *arg);

sgct::SGCTTrackingManager::SGCTTrackingManager()
{
    mHead = nullptr;
    mHeadUser = nullptr;
    mNumberOfDevices = 0;
    mSamplingThread = nullptr;
    mSamplingTime = 0.0;
    mRunning = true;
}

bool sgct::SGCTTrackingManager::isRunning()
{
    bool tmpVal;
#ifdef __SGCT_TRACKING_MUTEX_DEBUG__
    fprintf(stderr, "Checking if tracking is running...\n");
#endif
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );
        tmpVal = mRunning;
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );

    return tmpVal;
}

sgct::SGCTTrackingManager::~SGCTTrackingManager()
{
    MessageHandler::instance()->print(MessageHandler::NOTIFY_INFO, "Disconnecting VRPN...\n");

#ifdef __SGCT_TRACKING_MUTEX_DEBUG__
    fprintf(stderr, "Destructing, setting running to false...\n");
#endif
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );
        mRunning = false;
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );

    //destroy thread
    if( mSamplingThread != nullptr )
    {
        mSamplingThread->join();
        delete mSamplingThread;
        mSamplingThread = nullptr;
    }

    //delete all instances
    for(size_t i=0; i<mTrackers.size(); i++)
    {
        if( mTrackers[i] != nullptr )
        {
            delete mTrackers[i];
            mTrackers[i] = nullptr;
        }

        //clear vrpn pointers
        for(auto & mDevice : gTrackers[i].mDevices)
        {
            //clear sensor pointers
            if( mDevice.mSensorDevice != nullptr )
            {
                delete mDevice.mSensorDevice;
                mDevice.mSensorDevice = nullptr;
            }

            //clear analog pointers
            if( mDevice.mAnalogDevice != nullptr )
            {
                delete mDevice.mAnalogDevice;
                mDevice.mAnalogDevice = nullptr;
            }

            //clear button pointers
            if( mDevice.mButtonDevice != nullptr )
            {
                delete mDevice.mButtonDevice;
                mDevice.mButtonDevice = nullptr;
            }
        }
        gTrackers[i].mDevices.clear();
    }

    mTrackers.clear();
    gTrackers.clear();

    MessageHandler::instance()->print(MessageHandler::NOTIFY_DEBUG, "Done.\n");
}

void sgct::SGCTTrackingManager::startSampling()
{
    if( !mTrackers.empty() )
    {
        //find user with headtracking
        mHeadUser = sgct_core::ClusterManager::instance()->getTrackedUserPtr();

        //if tracked user not found
        if (mHeadUser == nullptr)
            mHeadUser = sgct_core::ClusterManager::instance()->getDefaultUserPtr();
        
        //link the head tracker
        setHeadTracker(mHeadUser->getHeadTrackerName(),
            mHeadUser->getHeadTrackerDeviceName());

        mSamplingThread = new std::thread( samplingLoop, this );
    }
}

/*
    Update the user position if headtracking is used. This function is called from the engine.
*/
void sgct::SGCTTrackingManager::updateTrackingDevices()
{
    for(SGCTTracker* tracker : mTrackers)
        for(size_t j=0; j<tracker->getNumberOfDevices(); j++)
        {
            SGCTTrackingDevice * tdPtr = tracker->getDevicePtr(j);
            if( tdPtr->isEnabled() && tdPtr == mHead && mHeadUser != nullptr)
            {
                mHeadUser->setTransform(tdPtr->getWorldTransform());
            }
        }
}

void sgct::SGCTTrackingManager::addTracker(std::string name)
{
    if (!getTrackerPtr(name.c_str()))
    {
        mTrackers.push_back(new SGCTTracker(name));

        VRPNTracker tmpVRPNTracker;
        gTrackers.push_back(tmpVRPNTracker);

        MessageHandler::instance()->print(MessageHandler::NOTIFY_INFO, "Tracking: Tracker '%s' added succesfully.\n", name.c_str());
    }
    else
        MessageHandler::instance()->print(MessageHandler::NOTIFY_WARNING, "Tracking: Tracker '%s' already exists!\n", name.c_str());
}

void sgct::SGCTTrackingManager::addDeviceToCurrentTracker(std::string name)
{
    mNumberOfDevices++;

    mTrackers.back()->addDevice(name, mTrackers.size() - 1);

    VRPNPointer tmpPtr;
    tmpPtr.mSensorDevice = nullptr;
    tmpPtr.mAnalogDevice = nullptr;
    tmpPtr.mButtonDevice = nullptr;

    gTrackers.back().mDevices.push_back( tmpPtr );
}

void sgct::SGCTTrackingManager::addSensorToCurrentDevice(const char * address, int id)
{
    if(gTrackers.empty() || gTrackers.back().mDevices.empty())
        return;

    std::pair<std::set<std::string>::iterator, bool> retVal =
        mAddresses.insert( std::string(address) );

    VRPNPointer * ptr = &gTrackers.back().mDevices.back();
    SGCTTrackingDevice * devicePtr = mTrackers.back()->getLastDevicePtr();

    if( devicePtr != nullptr)
    {
        devicePtr->setSensorId( id );

        if( retVal.second && (*ptr).mSensorDevice == nullptr)
        {
            MessageHandler::instance()->print(MessageHandler::NOTIFY_INFO, "Tracking: Connecting to sensor '%s'...\n", address);
            (*ptr).mSensorDevice = new vrpn_Tracker_Remote( address );

            if( (*ptr).mSensorDevice != nullptr )
            {
                (*ptr).mSensorDevice->register_change_handler( mTrackers.back(), update_tracker_cb);
            }
            else
                MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to connect to sensor '%s' on device %s!\n",
                    address, devicePtr->getName().c_str());
        }
    }
    else
        MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to connect to sensor '%s'!\n",
            address);
}

void sgct::SGCTTrackingManager::addButtonsToCurrentDevice(const char * address, size_t numOfButtons)
{
    if(gTrackers.empty() || gTrackers.back().mDevices.empty())
        return;

    VRPNPointer * ptr = &gTrackers.back().mDevices.back();
    SGCTTrackingDevice * devicePtr = mTrackers.back()->getLastDevicePtr();

    if( (*ptr).mButtonDevice == nullptr && devicePtr != nullptr)
    {
        MessageHandler::instance()->print(MessageHandler::NOTIFY_INFO, "Tracking: Connecting to buttons '%s' on device %s...\n",
                    address, devicePtr->getName().c_str());

        (*ptr).mButtonDevice = new vrpn_Button_Remote( address );

        if( (*ptr).mButtonDevice != nullptr ) //connected
        {
            (*ptr).mButtonDevice->register_change_handler(devicePtr, update_button_cb);
            devicePtr->setNumberOfButtons( numOfButtons );
        }
        else
            MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to connect to buttons '%s' on device %s!\n",
                address, devicePtr->getName().c_str());
    }
    else
        MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to connect to buttons '%s'!\n",
            address);
}

void sgct::SGCTTrackingManager::addAnalogsToCurrentDevice(const char * address, size_t numOfAxes)
{
    if(gTrackers.empty() || gTrackers.back().mDevices.empty())
        return;

    VRPNPointer * ptr = &gTrackers.back().mDevices.back();
    SGCTTrackingDevice * devicePtr = mTrackers.back()->getLastDevicePtr();

    if( (*ptr).mAnalogDevice == nullptr && devicePtr != nullptr)
    {
        MessageHandler::instance()->print(MessageHandler::NOTIFY_INFO, "Tracking: Connecting to analogs '%s' on device %s...\n",
                address, devicePtr->getName().c_str());

        (*ptr).mAnalogDevice = new vrpn_Analog_Remote( address );

        if( (*ptr).mAnalogDevice != nullptr )
        {
            (*ptr).mAnalogDevice->register_change_handler(devicePtr, update_analog_cb);
            devicePtr->setNumberOfAxes( numOfAxes );
        }
        else
            MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to connect to analogs '%s' on device %s!\n",
                address, devicePtr->getName().c_str());
    }
    else
        MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to connect to analogs '%s'!\n",
                address);
}

void sgct::SGCTTrackingManager::setHeadTracker(const char * trackerName, const char * deviceName)
{
    SGCTTracker * trackerPtr = getTrackerPtr( trackerName );

    if( trackerPtr != nullptr )
        mHead = trackerPtr->getDevicePtr( deviceName );
    //else no head tracker found

    if( mHead == nullptr && strlen(trackerName) > 0 && strlen(deviceName) > 0 )
        MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "Tracking: Failed to set head tracker to %s@%s!\n",
                deviceName, trackerName);
}

void samplingLoop(void *arg)
{
    auto * tmPtr =
        reinterpret_cast<sgct::SGCTTrackingManager *>(arg);

    double t;
    bool running = true;

    while(running)
    {
        t = sgct::Engine::getTime();
        for(size_t i=0; i<tmPtr->getNumberOfTrackers(); i++)
        {
            sgct::SGCTTracker * trackerPtr = tmPtr->getTrackerPtr(i);

            if( trackerPtr != nullptr )
            {
                for(size_t j=0; j<trackerPtr->getNumberOfDevices(); j++)
                {
                    if( trackerPtr->getDevicePtr(j)->isEnabled() )
                    {
                        if( gTrackers[i].mDevices[j].mSensorDevice != nullptr )
                            gTrackers[i].mDevices[j].mSensorDevice->mainloop();

                        if( gTrackers[i].mDevices[j].mAnalogDevice != nullptr )
                            gTrackers[i].mDevices[j].mAnalogDevice->mainloop();

                        if( gTrackers[i].mDevices[j].mButtonDevice != nullptr )
                            gTrackers[i].mDevices[j].mButtonDevice->mainloop();
                    }
                }
            }
        }

        running = tmPtr->isRunning();

        tmPtr->setSamplingTime(sgct::Engine::getTime() - t);

        // Sleep for 1ms so we don't eat the CPU
        vrpn_SleepMsecs(1);
    }
}

sgct::SGCTTracker * sgct::SGCTTrackingManager::getLastTrackerPtr()
{
    return !mTrackers.empty() ? mTrackers.back() : nullptr;
}

sgct::SGCTTracker * sgct::SGCTTrackingManager::getTrackerPtr(size_t index)
{
    return index < mTrackers.size() ? mTrackers[index] : nullptr;
}

sgct::SGCTTracker * sgct::SGCTTrackingManager::getTrackerPtr(const char * name)
{
    for(SGCTTracker* tracker : mTrackers)
    {
        if( strcmp(name, tracker->getName().c_str()) == 0 )
            return tracker;
    }

    //MessageHandler::instance()->print(MessageHandler::NOTIFY_ERROR, "SGCTTrackingManager: Tracker '%s' not found!\n", name);

    //if not found
    return nullptr;
}

void sgct::SGCTTrackingManager::setEnabled(bool state)
{
    for(SGCTTracker* tracker : mTrackers)
    {
        tracker->setEnabled( state );
    }
}

void sgct::SGCTTrackingManager::setSamplingTime(double t)
{
#ifdef __SGCT_TRACKING_MUTEX_DEBUG__
    fprintf(stderr, "Set sampling time for vrpn loop...\n");
#endif
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );
        mSamplingTime = t;
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );
}

double sgct::SGCTTrackingManager::getSamplingTime()
{
#ifdef __SGCT_TRACKING_MUTEX_DEBUG__
    fprintf(stderr, "Get sampling time for vrpn loop...\n");
#endif
    double tmpVal;
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );
        tmpVal = mSamplingTime;
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );

    return tmpVal;
}

void VRPN_CALLBACK update_tracker_cb(void *userdata, const vrpn_TRACKERCB info)
{
    auto * trackerPtr =
        reinterpret_cast<sgct::SGCTTracker *>(userdata);
    if( trackerPtr == nullptr )
        return;

    sgct::SGCTTrackingDevice * devicePtr = trackerPtr->getDevicePtrBySensorId( info.sensor );

    if(devicePtr == nullptr)
        return;

    glm::dvec3 posVec = glm::dvec3( info.pos[0], info.pos[1], info.pos[2] );
    posVec *= trackerPtr->getScale();

    glm::dquat rotation(info.quat[3], info.quat[0], info.quat[1], info.quat[2]);

    devicePtr->setSensorTransform(posVec, rotation);
}

void VRPN_CALLBACK update_button_cb(void *userdata, const vrpn_BUTTONCB b )
{
    auto * devicePtr =
        reinterpret_cast<sgct::SGCTTrackingDevice *>(userdata);

    //fprintf(stderr, "Button: %d, state: %d\n", b.button, b.state);

    b.state == 0 ?
        devicePtr->setButtonVal( false, b.button) :
        devicePtr->setButtonVal( true, b.button);
}

void VRPN_CALLBACK update_analog_cb(void* userdata, const vrpn_ANALOGCB a )
{
    auto * tdPtr =
        reinterpret_cast<sgct::SGCTTrackingDevice *>(userdata);
    tdPtr->setAnalogVal( a.channel, static_cast<size_t>(a.num_channel));
}
