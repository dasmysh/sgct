/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#include <sgct/ogl_headers.h>
#include <sgct/ShaderManager.h>
#include <sgct/MessageHandler.h>

#include <algorithm>

// Initiate the manager to NULL
sgct::ShaderManager * sgct::ShaderManager::mInstance = nullptr;

/*!
Default constructor does nothing only sets the NULL value shader program
that can be used for comparison will NULL value shader programs
*/
sgct::ShaderManager::ShaderManager() :
    NullShader( "SGCT_NULL" )
{
    mCurrentBin = SHADER_BIN_0;
}
//----------------------------------------------------------------------------//

/*!
Destructor deallocates and deletes all shaders
*/
sgct::ShaderManager::~ShaderManager()
{
    for(auto & mShaderProgram : mShaderPrograms)
    {
        auto it = mShaderProgram.begin();
        auto end = mShaderProgram.end();

        for( ; it != end; ++it )
        {
            it->deleteProgram();
        }

        mShaderProgram.clear();
    }
}
//----------------------------------------------------------------------------//

/*!
Add a empty shader program to the manager. This function is used when creating advanced shader programs. Compilation is not automatic in this case.
@param    name        Unique name of the shader
@param    shaderProgram    Reference to ShaderProgram
@return true if shader program was added to the shader manager
*/
bool sgct::ShaderManager::addShaderProgram( const std::string & name, ShaderProgram & shaderProgram )
{
    //
    // Check if shader already exists
    //
    if( shaderProgramExists( name ) )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to add shader program [%s]: Name already exists.\n", name.c_str() );
        return false;
    }

    //
    // If shader don't exist, create it and add to container
    //

    ShaderProgram sp( name );
    
    mShaderPrograms[mCurrentBin].push_back( sp );
    shaderProgram = mShaderPrograms[mCurrentBin].back();

    return true;
}

//----------------------------------------------------------------------------//

/*!
Add a shader program to the manager. The shaders will be compiled and linked to
the program. The name of the shader needs to be unique or it won't be added. 
Both vertex shader and fragment shader source need to be provided, either as a 
link to a shader source code file or as shader source code.
@param    name        Unique name of the shader
@param    vertexSrc    The vertex shader source code, can be a file path or source code
@param    fragmentSrc    The fragment shader source code, can be a file path or source code
@param    sSrcTyp        Shader source code type, if it is a link to a file or source code
@return    Wether the shader was created, linked and added to the manager correctly or not.
*/
bool sgct::ShaderManager::addShaderProgram( 
    const std::string & name,
    const std::string & vertexSrc,
    const std::string & fragmentSrc,
    sgct::ShaderProgram::ShaderSourceType sSrcType )
{
    //
    // Check if shader already exists
    //
    if( shaderProgramExists( name ) )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to add shader program [%s]: Name already exists.\n", name.c_str() );
        return false;
    }

    //
    // If shader don't exist, create it and add to container
    //

    ShaderProgram sp( name );
    
    if( !sp.addShaderSrc( vertexSrc, GL_VERTEX_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }
    
    if( !sp.addShaderSrc( fragmentSrc, GL_FRAGMENT_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }

    if( sp.createAndLinkProgram() )
    {
        mShaderPrograms[mCurrentBin].push_back( sp );
        return true;
    }

    // If arrived here the creation and linking of the program didn't work.
    // Return false but printing errors is handled in createAndLinkProgram()
    return false;
}
//----------------------------------------------------------------------------//

/*!
Add a shader program to the manager. The shaders will be compiled and linked to
the program. The name of the shader needs to be unique or it won't be added. 
Both vertex shader and fragment shader source need to be provided, either as a 
link to a shader source code file or as shader source code.
@param    shaderProgram    Reference to ShaderProgram
@param    name        Unique name of the shader
@param    vertexSrc    The vertex shader source code, can be a file path or source code
@param    fragmentSrc    The fragment shader source code, can be a file path or source code
@param    sSrcTyp        Shader source code type, if it is a link to a file or source code
@return    Wether the shader was created, linked and added to the manager correctly or not.
*/
bool sgct::ShaderManager::addShaderProgram( 
    ShaderProgram & shaderProgram,
    const std::string & name,
    const std::string & vertexSrc,
    const std::string & fragmentSrc,
    sgct::ShaderProgram::ShaderSourceType sSrcType )
{
    //if something failes set shader pointer to NullShader
    shaderProgram = NullShader;
    
    //
    // Check if shader already exists
    //
    if( shaderProgramExists( name ) )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to add shader program [%s]: Name already exists.\n", name.c_str() );
        return false;
    }

    ShaderProgram sp( name );
    
    if( !sp.addShaderSrc( vertexSrc, GL_VERTEX_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }
    
    if( !sp.addShaderSrc( fragmentSrc, GL_FRAGMENT_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }

    if( sp.createAndLinkProgram() )
    {
        mShaderPrograms[mCurrentBin].push_back( sp );
        shaderProgram = mShaderPrograms[mCurrentBin].back();
        return true;
    }

    // If arrived here the creation and linking of the program didn't work.
    // Return false but printing errors is handled in createAndLinkProgram()
    return false;
}
//----------------------------------------------------------------------------//

/*!
Add a shader program to the manager. The shaders will be compiled and linked to
the program. The name of the shader needs to be unique or it won't be added. 
Both vertex shader and fragment shader source need to be provided, either as a 
link to a shader source code file or as shader source code.
@param    name        Unique name of the shader
@param    vertexSrc    The vertex shader source code, can be a file path or source code
@param    fragmentSrc    The fragment shader source code, can be a file path or source code
@param    geometrySrc    The geometry shader source code, can be a file path or source code
@param    sSrcTyp        Shader source code type, if it is a link to a file or source code
@return    Wether the shader was created, linked and added to the manager correctly or not.
*/
bool sgct::ShaderManager::addShaderProgram( 
    const std::string & name,
    const std::string & vertexSrc,
    const std::string & fragmentSrc,
    const std::string & geometrySrc,
    sgct::ShaderProgram::ShaderSourceType sSrcType )
{
    //
    // Check if shader already exists
    //
    if( shaderProgramExists( name ) )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to add shader program [%s]: Name already exists.\n", name.c_str() );
        return false;
    }

    //
    // If shader don't exist, create it and add to container
    //
    ShaderProgram sp( name );
    
    if( !sp.addShaderSrc( vertexSrc, GL_VERTEX_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }
    
    if( !sp.addShaderSrc( fragmentSrc, GL_FRAGMENT_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }

    if( !sp.addShaderSrc( geometrySrc, GL_GEOMETRY_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }

    if( sp.createAndLinkProgram() )
    {
        mShaderPrograms[mCurrentBin].push_back( sp );
        return true;
    }

    // If arrived here the creation and linking of the program didn't work.
    // Return false but printing errors is handled in createAndLinkProgram()
    return false;
}
//----------------------------------------------------------------------------//

/*!
Add a shader program to the manager. The shaders will be compiled and linked to
the program. The name of the shader needs to be unique or it won't be added. 
Both vertex shader and fragment shader source need to be provided, either as a 
link to a shader source code file or as shader source code.
@param    shaderProgram    Reference to ShaderProgram
@param    name        Unique name of the shader
@param    vertexSrc    The vertex shader source code, can be a file path or source code
@param    fragmentSrc    The fragment shader source code, can be a file path or source code
@param    geometrySrc    The geometry shader source code, can be a file path or source code
@param    sSrcTyp        Shader source code type, if it is a link to a file or source code
@return    Wether the shader was created, linked and added to the manager correctly or not.
*/
bool sgct::ShaderManager::addShaderProgram( 
    ShaderProgram & shaderProgram,
    const std::string & name,
    const std::string & vertexSrc,
    const std::string & fragmentSrc,
    const std::string & geometrySrc,
    sgct::ShaderProgram::ShaderSourceType sSrcType )
{
    //if something failes set shader pointer to NullShader
    shaderProgram = NullShader;
    
    //
    // Check if shader already exists
    //
    if( shaderProgramExists( name ) )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to add shader program [%s]: Name already exists.\n", name.c_str() );
        return false;
    }

    //
    // If shader don't exist, create it and add to container
    //
    ShaderProgram sp( name );
    
    if( !sp.addShaderSrc( vertexSrc, GL_VERTEX_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }
    
    if( !sp.addShaderSrc( fragmentSrc, GL_FRAGMENT_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }

    if( !sp.addShaderSrc( geometrySrc, GL_GEOMETRY_SHADER, sSrcType ) )
    {
        // Error messaging handled when setting source
        return false;
    }

    if( sp.createAndLinkProgram() )
    {
        mShaderPrograms[mCurrentBin].push_back( sp );
        shaderProgram = mShaderPrograms[mCurrentBin].back();
        return true;
    }

    // If arrived here the creation and linking of the program didn't work.
    // Return false but printing errors is handled in createAndLinkProgram()
    return false;
}
//----------------------------------------------------------------------------//

/*!
    Select which shader bin to use in the manager.

    \param bin the bin to use in the shader manager
*/
void sgct::ShaderManager::setCurrentBin( ShaderBinIndex bin )
{
    mCurrentBin = bin;
}

/*!
Reloads a shader program from the manager for the current bin.
@param    name    Name of the shader program to reload
@return    true if the shader program was reloaded correctly
*/
bool sgct::ShaderManager::reloadShaderProgram( const std::string & name )
{
    auto shaderIt = std::find( mShaderPrograms[mCurrentBin].begin(), mShaderPrograms[mCurrentBin].end(), name );

    if( shaderIt == mShaderPrograms[mCurrentBin].end() )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to reload shader program [%s]: Not found in current bin.\n", name.c_str() );
        return false;
    }

    shaderIt->reload();

    return true;
}

/*!
Removes a shader program from the manager for the current bin.
All resources allocated for the program will be deallocated and removed
@param    name    Name of the shader program to remove
@return    true if the shader program was removed correctly
*/
bool sgct::ShaderManager::removeShaderProgram( const std::string & name )
{
    auto shaderIt = std::find( mShaderPrograms[mCurrentBin].begin(), mShaderPrograms[mCurrentBin].end(), name );

    if( shaderIt == mShaderPrograms[mCurrentBin].end() )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to remove shader program [%s]: Not found in current bin.\n", name.c_str() );
        return false;
    }

    shaderIt->deleteProgram();
    mShaderPrograms[mCurrentBin].erase( shaderIt );

    return true;
}
//----------------------------------------------------------------------------//

/*!
Removes a shader program from the manager for the selected bin.
All resources allocated for the program will be deallocated and remved
@param    name    Name of the shader program to remove
@param    bin        Which bin to remove the shader from
@return    true if the shader program was removed correctly
*/
bool sgct::ShaderManager::removeShaderProgram( const std::string & name, ShaderBinIndex bin )
{
    auto shaderIt = std::find( mShaderPrograms[bin].begin(), mShaderPrograms[bin].end(), name );

    if( shaderIt == mShaderPrograms[bin].end() )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Unable to remove shader program [%s]: Not found in current bin.\n", name.c_str() );
        return false;
    }

    shaderIt->deleteProgram();
    mShaderPrograms[mCurrentBin].erase( shaderIt );

    return true;
}
//----------------------------------------------------------------------------//

/*!
Set a shader program to be used in the current rendering pipeline
@param    name    Name of the shader program to set as active
@return    Wether the specified shader was set as active or not.
*/
bool sgct::ShaderManager::bindShaderProgram( const std::string & name ) const
{
    ShaderProgram sp = getShaderProgram( name );

    if( sp == NullShader )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Could not set shader program [%s] as active: Not found in manager.\n", name.c_str() );
        glUseProgram( GL_FALSE ); //unbind to prevent errors
        return false;
    }

    return sp.bind();
}
//----------------------------------------------------------------------------//

/*!
Set a shader program to be used in the current rendering pipeline
@param    shader    Reference to the shader program to set as active
@return    Wether the specified shader was set as active or not.
*/
bool sgct::ShaderManager::bindShaderProgram( const ShaderProgram & shaderProgram ) const
{
    if( shaderProgram == NullShader )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_WARNING, "Could not set shader program [Invalid Pointer] as active: Not found in manager.\n");
        glUseProgram( GL_FALSE ); //unbind to prevent errors
        return false;
    }

    return shaderProgram.bind();
}
//----------------------------------------------------------------------------//

/*!
    Unbind/unset/diable current shader program in the rendering pipeline.
*/
void sgct::ShaderManager::unBindShaderProgram()
{
    glUseProgram( GL_FALSE );
}
//----------------------------------------------------------------------------//
/*!
Get the specified shader program from the shader manager. If the shader is not found
ShaderManager::NullShader will be returned which can be used for comparisons. The NullShader
can not be set as active or used in the rendering pipeline
@param    name    Name of the shader program
@return The specified shader program or ShaderManager::NullShader if shader is not found.
*/
const sgct::ShaderProgram & sgct::ShaderManager::getShaderProgram( const std::string & name ) const
{
    auto shaderIt = std::find( mShaderPrograms[mCurrentBin].begin(), mShaderPrograms[mCurrentBin].end(), name );

    return (shaderIt != mShaderPrograms[mCurrentBin].end() ) ?
        *shaderIt :
        NullShader;
}

//----------------------------------------------------------------------------//

/*!
Check if a shader program exists in the manager
@param    name    Name of the shader program.
*/
bool sgct::ShaderManager::shaderProgramExists( const std::string & name ) const
{
    auto exists = std::find( mShaderPrograms[mCurrentBin].begin(), mShaderPrograms[mCurrentBin].end(), name );

    return exists != mShaderPrograms[mCurrentBin].end();
}
//----------------------------------------------------------------------------//
