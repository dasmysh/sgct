/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _SGCT_MUTEX_MANAGER
#define _SGCT_MUTEX_MANAGER

#define SGCT_NUMBER_OF_MUTEXES 6

#include <mutex>
#include <stddef.h>

namespace sgct
{

/*!
    This singleton class manages SGCTs mutexes
*/
class SGCTMutexManager
{
public:
    enum MutexIndexes { DataSyncMutex = 0, FrameSyncMutex, TrackingMutex, ConsoleMutex, TransferMutex };

    /*! Get the SGCTSettings instance */
    static SGCTMutexManager * instance()
    {
        if( mInstance == nullptr )
        {
            mInstance = new SGCTMutexManager();
        }

        return mInstance;
    }

    /*! Destroy the SGCTSettings instance */
    static void destroy()
    {
        if( mInstance != nullptr )
        {
            delete mInstance;
            mInstance = nullptr;
        }
    }

    void lockMutex(MutexIndexes mi);
    void unlockMutex(MutexIndexes mi);
    std::mutex * getMutexPtr(MutexIndexes mi);

private:
    SGCTMutexManager();
    ~SGCTMutexManager();

    SGCTMutexManager( const SGCTMutexManager & settings ) = delete;
    const SGCTMutexManager & operator=(const SGCTMutexManager & settings ) = delete;

private:
    static SGCTMutexManager * mInstance;
    std::mutex mInternalMutexes[SGCT_NUMBER_OF_MUTEXES];
};
}

#endif