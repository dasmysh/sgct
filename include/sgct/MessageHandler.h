/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _MESSAGE_HANDLER
#define _MESSAGE_HANDLER

#include <stddef.h> //get definition for NULL
#include <stdarg.h>
#include <vector>
#include <string>
#include "helpers/SGCTCPPEleven.h"

#include <atomic>

#define TIME_BUFFER_SIZE 9
#define LOG_FILENAME_BUFFER_SIZE 1024 //include path

namespace sgct //simple graphics cluster toolkit
{

class MessageHandler
{
public:
    /*!
        Different notify levels for messages
    */
    enum NotifyLevel { NOTIFY_ERROR = 0, NOTIFY_IMPORTANT, NOTIFY_VERSION_INFO, NOTIFY_INFO, NOTIFY_WARNING, NOTIFY_DEBUG, NOTIFY_ALL };
    
    /*! Get the MessageHandler instance */
    static MessageHandler * instance()
    {
        if( mInstance == nullptr )
        {
            mInstance = new MessageHandler();
        }

        return mInstance;
    }

    /*! Destroy the MessageHandler */
    static void destroy()
    {
        if( mInstance != nullptr )
        {
            delete mInstance;
            mInstance = nullptr;
        }
    }

    void decode(const char * receivedData, int receivedlength, int clientIndex);
    void print(const char *fmt, ...);
    void print(NotifyLevel nl, const char *fmt, ...);
    void printDebug(NotifyLevel nl, const char *fmt, ...);
    void printIndent(NotifyLevel nl, unsigned int indentation, const char* fmt, ...);
    void sendMessageToServer(const char *fmt);
    void setSendFeedbackToServer(bool state);
    void clearBuffer();
    void setNotifyLevel( NotifyLevel nl );
    NotifyLevel getNotifyLevel();
    void setShowTime( bool state );
    bool getShowTime();
    void setLogToConsole( bool state );
    void setLogToFile( bool state );
    void setLogPath(const char * path, int nodeId = -1);
    void setLogToCallback( bool state );
    void setLogCallback(void(*fnPtr)(const char *));
#ifdef __LOAD_CPP11_FUN__
    void setLogCallback(sgct_cppxeleven::function<void(const char *)> fn);
#endif
    const char * getTimeOfDayStr();
    inline std::size_t getDataSize() { return mBuffer.size(); }

    char * getMessage();

private:
    MessageHandler();
    ~MessageHandler();

    MessageHandler( const MessageHandler & tm ) = delete;
    const MessageHandler & operator=(const MessageHandler & rhs) = delete;

    // Don't implement these, should give compile warning if used
    void printv(const char *fmt, va_list ap);
    void logToFile(const char * buffer);

private:
#ifdef __LOAD_CPP11_FUN__
    using MessageCallbackFn = sgct_cppxeleven::function<void(const char *)>;
#else
    typedef void(*MessageCallbackFn)(const char *);
#endif

    static MessageHandler * mInstance;

    char * mParseBuffer;
    char * mCombinedBuffer;
    
    std::vector<char> mBuffer;
    std::vector<char> mRecBuffer;
    unsigned char  * headerSpace;

    std::atomic<int> mLevel;
    std::atomic<bool> mLocal;
    std::atomic<bool> mShowTime;
    std::atomic<bool> mLogToConsole;
    std::atomic<bool> mLogToFile;
    std::atomic<bool> mLogToCallback;

    MessageCallbackFn mMessageCallback;
    char mTimeBuffer[TIME_BUFFER_SIZE];
    std::string mFilename;
    size_t mMaxMessageSize;
    size_t mCombinedMessageSize;
};

}

#endif
