/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#include <sgct/SGCTNode.h>
#include <sgct/MessageHandler.h>
#include <algorithm>

sgct_core::SGCTNode::SGCTNode()
{
    mCurrentWindowIndex = 0;
    mUseSwapGroups = false;
}

/*!
    Add a window to the window vector. Note that a window must be opened to become visible.
*/
void sgct_core::SGCTNode::addWindow(sgct::SGCTWindow window)
{
    mWindows.push_back(window);
}

/*!
    Set which window that will render the draw calls.
*/
void sgct_core::SGCTNode::setCurrentWindowIndex(std::size_t index)
{
    mCurrentWindowIndex = index;
}

/*!
    Set to true if this node's windows should belong to a nvida swap group. Only valid before window opens.
*/
void sgct_core::SGCTNode::setUseSwapGroups(bool state)
{
    mUseSwapGroups = state;
}

/*!
    Check if a key is pressed for all windows.
*/
bool sgct_core::SGCTNode::getKeyPressed( int key )
{
    if (key == GLFW_KEY_UNKNOWN)
        return false;

    for(sgct::SGCTWindow & window : mWindows)
        if( glfwGetKey(window.getWindowHandle(), key) )
            return true;
    return false;
}

/*!
    Check if all windows are set to close and close them.
*/
bool sgct_core::SGCTNode::shouldAllWindowsClose()
{
    std::size_t counter = 0;
    for(sgct::SGCTWindow & window : mWindows)
        if( glfwWindowShouldClose(window.getWindowHandle() ) )
        {
            window.setVisibility( false );
            glfwSetWindowShouldClose(window.getWindowHandle(), GL_FALSE );
        }

    for(sgct::SGCTWindow & window : mWindows)
    if (!(window.isVisible() || window.isRenderingWhileHidden()))
    //if (!mWindows[i].isVisible())
        {
            counter++;
        }

    return (counter == mWindows.size()) ? true : false;
}

/*!
    Show all hidden windows.
*/
void sgct_core::SGCTNode::showAllWindows()
{
    for(sgct::SGCTWindow & window : mWindows)
        window.setVisibility( true );
}

/*!
    Is this node using nvidia swap groups for it's windows?
*/
bool sgct_core::SGCTNode::isUsingSwapGroups()
{
    return mUseSwapGroups;
}

/*!
    Hide all windows.
*/
void sgct_core::SGCTNode::hideAllWindows()
{
    for(sgct::SGCTWindow & window : mWindows)
        window.setVisibility( false );
}

/*!
\param address is the hostname, DNS-name or ip
*/
void sgct_core::SGCTNode::setAddress(std::string address)
{
    std::transform(address.begin(), address.end(), address.begin(), ::tolower);
    mAddress.assign( address );

    sgct::MessageHandler::instance()->print( sgct::MessageHandler::NOTIFY_DEBUG,
        "SGCTNode: Setting address to %s\n", mAddress.c_str() );
}

/*!
\param sync port is the number of the tcp port used for communication with this node
*/
void sgct_core::SGCTNode::setSyncPort(std::string port)
{
    mSyncPort.assign( port );
    
    sgct::MessageHandler::instance()->print( sgct::MessageHandler::NOTIFY_DEBUG,
        "SGCTNode: Setting sync port to %s\n", mSyncPort.c_str());
}

/*!
\param data transfer port is the number of the tcp port used for data transfers to this node
*/
void sgct_core::SGCTNode::setDataTransferPort(std::string port)
{
    mDataTransferPort.assign(port);

    sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG,
        "SGCTNode: Setting data transfer port to %s\n", mDataTransferPort.c_str());
}

/*!
\param name the name identification string of this node
*/
void sgct_core::SGCTNode::setName(std::string name)
{
    mName = name;
}

/*!
\returns the address of this node
*/
std::string sgct_core::SGCTNode::getAddress()
{
    return mAddress;
}

/*!
\returns the sync port of this node
*/
std::string sgct_core::SGCTNode::getSyncPort()
{
    return mSyncPort;
}

/*!
\returns the data transfer port of this node
*/
std::string sgct_core::SGCTNode::getDataTransferPort()
{
    return mDataTransferPort;
}

/*!
\returns the name if this node
*/
std::string sgct_core::SGCTNode::getName()
{
    return mName;
}
