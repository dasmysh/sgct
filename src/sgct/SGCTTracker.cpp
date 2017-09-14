/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#include <sgct/SGCTTracker.h>
#include <sgct/MessageHandler.h>
#include <sgct/Engine.h>

extern GLFWmutex gTrackingMutex;

sgct::SGCTTracker::SGCTTracker(std::string name)
{
    mName.assign(name);

    mXform = glm::mat4(1.0f);
    mOffset = glm::vec3(0.0f);

    mScale = 1.0;
}

sgct::SGCTTracker::~SGCTTracker()
{
    for(auto & mTrackingDevice : mTrackingDevices)
    {
        if( mTrackingDevice != nullptr )
        {
            delete mTrackingDevice;
            mTrackingDevice = nullptr;
        }
    }

    mTrackingDevices.clear();
}

void sgct::SGCTTracker::setEnabled(bool state)
{
    for(auto & mTrackingDevice : mTrackingDevices)
    {
        mTrackingDevice->setEnabled( state );
    }
}

void sgct::SGCTTracker::addDevice(std::string name, size_t index)
{
    SGCTTrackingDevice * td = new SGCTTrackingDevice( index, name );

    mTrackingDevices.push_back( td );

    MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_INFO, "%s: Adding device '%s'...\n", mName.c_str(), name.c_str());
}

sgct::SGCTTrackingDevice * sgct::SGCTTracker::getLastDevicePtr()
{
    return !mTrackingDevices.empty() ? mTrackingDevices.back() : nullptr;
}

sgct::SGCTTrackingDevice * sgct::SGCTTracker::getDevicePtr(size_t index)
{
    return index < mTrackingDevices.size() ? mTrackingDevices[index] : nullptr;
}

sgct::SGCTTrackingDevice * sgct::SGCTTracker::getDevicePtr(const char * name)
{
    for(auto & mTrackingDevice : mTrackingDevices)
    {
        if( strcmp(name, mTrackingDevice->getName().c_str()) == 0 )
            return mTrackingDevice;
    }

    //if not found
    return nullptr;
}

sgct::SGCTTrackingDevice * sgct::SGCTTracker::getDevicePtrBySensorId(int id)
{
    for(auto & mTrackingDevice : mTrackingDevices)
    {
        if( mTrackingDevice->getSensorId() == id )
            return mTrackingDevice;
    }

    return nullptr;
}

/*!
Set the orientation as quaternion
*/
void sgct::SGCTTracker::setOrientation(glm::quat q)
{
    SGCTMutexManager::instance()->lockMutex(SGCTMutexManager::TrackingMutex);

    //create inverse rotation matrix
    mOrientation = glm::inverse(glm::mat4_cast(q));

    calculateTransform();
    SGCTMutexManager::instance()->unlockMutex(SGCTMutexManager::TrackingMutex);
}

/*!
Set the orientation as euler angles (degrees)
*/
void sgct::SGCTTracker::setOrientation(float xRot, float yRot, float zRot)
{
    //create rotation quaternion based on x, y, z rotations
    glm::quat rotQuat;
    rotQuat = glm::rotate(rotQuat, glm::radians(xRot), glm::vec3(1.0f, 0.0f, 0.0f));
    rotQuat = glm::rotate(rotQuat, glm::radians(yRot), glm::vec3(0.0f, 1.0f, 0.0f));
    rotQuat = glm::rotate(rotQuat, glm::radians(zRot), glm::vec3(0.0f, 0.0f, 1.0f));
    
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );

    //create inverse rotation matrix
    mOrientation = glm::inverse( glm::mat4_cast(rotQuat) );

    calculateTransform();
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );
}

/*!
Set the orientation as a quaternion
*/
void sgct::SGCTTracker::setOrientation(float w, float x, float y, float z)
{
    glm::quat rotQuat(w, x, y, z);
    SGCTMutexManager::instance()->lockMutex(SGCTMutexManager::TrackingMutex);
    
    //create inverse rotation matrix
    mOrientation = glm::inverse(glm::mat4_cast(rotQuat));

    calculateTransform();
    SGCTMutexManager::instance()->unlockMutex(SGCTMutexManager::TrackingMutex);
}

void sgct::SGCTTracker::setOffset(float x, float y, float z)
{
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );
        mOffset[0] = x;
        mOffset[1] = y;
        mOffset[2] = z;

        calculateTransform();
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );
}

void sgct::SGCTTracker::setScale(double scaleVal)
{
    SGCTMutexManager::instance()->lockMutex( SGCTMutexManager::TrackingMutex );
        if( scaleVal > 0.0 )
            mScale = scaleVal;
    SGCTMutexManager::instance()->unlockMutex( SGCTMutexManager::TrackingMutex );
}

/*
Set the tracker system transform matrix\n
worldTransform = (trackerTransform * sensorMat) * deviceTransformMat
*/
void sgct::SGCTTracker::setTransform(glm::mat4 mat)
{
    SGCTMutexManager::instance()->lockMutex(SGCTMutexManager::TrackingMutex);
    mXform = mat;
    SGCTMutexManager::instance()->unlockMutex(SGCTMutexManager::TrackingMutex);
}

void sgct::SGCTTracker::calculateTransform()
{
    //create offset translation matrix
    glm::mat4 transMat = glm::translate(glm::mat4(1.0f), mOffset);
    
    //calculate transform
    mXform = transMat * mOrientation;
}

glm::mat4 sgct::SGCTTracker::getTransform()
{ 
    glm::mat4 tmpMat;
    SGCTMutexManager::instance()->lockMutex(SGCTMutexManager::TrackingMutex);
    tmpMat = mXform;
    SGCTMutexManager::instance()->unlockMutex(SGCTMutexManager::TrackingMutex);
    return tmpMat;
}

double sgct::SGCTTracker::getScale()
{
    double tmpD;
    SGCTMutexManager::instance()->lockMutex(SGCTMutexManager::TrackingMutex);
    tmpD = mScale;
    SGCTMutexManager::instance()->unlockMutex(SGCTMutexManager::TrackingMutex);
    return tmpD;
}
