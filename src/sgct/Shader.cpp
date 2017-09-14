/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#include <sgct/ogl_headers.h>
#include <sgct/Shader.h>
#include <sgct/MessageHandler.h>

#include <fstream>
#include <sstream>
#include <iostream>

/*!
Default constructor
*/
sgct_core::Shader::Shader() : 
    mShaderId( 0 )
{
}
//----------------------------------------------------------------------------//

/*!
The constructor sets shader type
@param    shaderType    The shader type: GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER
*/
sgct_core::Shader::Shader( sgct_core::Shader::ShaderType shaderType ) :
    mShaderType( shaderType ),
    mShaderId( 0 )
{
}
//----------------------------------------------------------------------------//

/*!
Destructor does nothing, have to explicitly call delete if shader should be destroyed.
This is because copying between shaders should be allowed (i.e. when storing in containers)
*/
sgct_core::Shader::~Shader() = default;
//----------------------------------------------------------------------------//

/*!
Set the shader type
@param    shaderType    The shader type: GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, or GL_FRAGMENT_SHADER
*/
void sgct_core::Shader::setShaderType( ShaderType shaderType )
{
    mShaderType = shaderType;
}

/*!
Set the shader source code from a file, will create and compile the shader if it is not already done.
At this point a compiled shader can't have its source reset. Recompilation of shaders is not supported
@param    file    Path to shader file
@return    If setting source and compilation went ok.
*/
bool sgct_core::Shader::setSourceFromFile( const std::string & file )
{
    //
    // Make sure file can be opened
    //
    std::ifstream shaderFile( file.c_str() );

    if( !shaderFile.is_open() )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, 
            "Could not open %s file[%s].\n",
            getShaderTypeName( mShaderType ).c_str(),
            file.c_str() );
        return false;
    }

    //
    // Create needed resources by reading file length
    //
    shaderFile.seekg( 0, std::ios_base::end );
    std::streamoff fileLength = shaderFile.tellg();

    shaderFile.seekg( 0, std::ios_base::beg );

    //
    // Make sure the file is not empty
    //
    if( fileLength == 0 )
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, 
            "Can't create source for %s: empty file [%s].\n",
            getShaderTypeName( mShaderType ).c_str(),
            file.c_str() );
        return false;
    }

    //
    // Copy file content to string
    //

    // Obs: take special care if changing the way of reading the file.
    // This was the only way I got it to work for both VS2010 and GCC 4.6.2 without
    // crashing. See the commented lines below for how it originally was. Those
    // lines did not work with GCC 4.6.2. Feel free to update the reading but
    // make sure it works for both VS and GCC.

    std::string shaderSrc;
    shaderSrc.reserve(4096);
    while ( shaderFile.good() )
    {
        char c = shaderFile.get();

        if( shaderFile.good() ) shaderSrc += c;
    }

    //std::string shaderSrc( fileLength, '\0' );
    //shaderFile.read( &shaderSrc[0], fileLength );

    shaderFile.close();

    //
    // Compile shader source
    //
    return setSourceFromString( shaderSrc );
}
//----------------------------------------------------------------------------//

/*!
Set the shader source code from a file, will create and compile the shader if it is not already done.
At this point a compiled shader can't have its source reset. Recompilation of shaders is not supported
@param    sourceString    String with shader source code
@return    If setting the source and compilation went ok.
*/
bool sgct_core::Shader::setSourceFromString( const std::string & sourceString )
{
    //
    // At this point no resetting of shaders are supported
    //
    if( mShaderId > 0 )
    {
        sgct::MessageHandler::instance()->print(
            sgct::MessageHandler::NOTIFY_WARNING, 
            "%s is already set for specified shader.\n",
            getShaderTypeName( mShaderType ).c_str() );
        return false;
    }

    //
    // Prepare source code for shader
    //
    const char * shaderSrc[] = { sourceString.c_str() };

    mShaderId = glCreateShader( mShaderType );
    glShaderSource( mShaderId, 1, shaderSrc, nullptr );

    //
    // Compile and check status
    //
    glCompileShader( mShaderId );

    return checkCompilationStatus();
}
//----------------------------------------------------------------------------//

/*! Delete the shader */
void sgct_core::Shader::deleteShader()
{
    glDeleteShader( mShaderId );
    mShaderId = GL_FALSE;
}
//----------------------------------------------------------------------------//

/*!
Will check the compilation status of the shader and output any errors from the shader log
return    Status of the compilation
*/
bool sgct_core::Shader::checkCompilationStatus() const
{
    GLint compilationStatus;
    glGetShaderiv( mShaderId, GL_COMPILE_STATUS, &compilationStatus );

    if( compilationStatus == GL_FALSE )
       {
        GLint logLength;
        glGetShaderiv( mShaderId, GL_INFO_LOG_LENGTH, &logLength );

        if( logLength == 0 )
        {
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "%s compile error: Unknown error\n", getShaderTypeName( mShaderType ).c_str() );
            return false;
        }

        auto * log = new GLchar[logLength];

        glGetShaderInfoLog( mShaderId, logLength, nullptr, log );
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "%s compile error: %s\n", getShaderTypeName( mShaderType ).c_str(), log );

        delete[] log;

        return false;;
    }

    return compilationStatus == GL_TRUE;
}
//----------------------------------------------------------------------------//

/*!
Will return the name of the shader type
@param    shaderType    The shader type
@return    Shader type name
*/
std::string sgct_core::Shader::getShaderTypeName( ShaderType shaderType ) const
{
    switch( shaderType )
    {
    case GL_VERTEX_SHADER:
        return "Vertex shader";
    case GL_FRAGMENT_SHADER:
        return "Fragment shader";
    case GL_GEOMETRY_SHADER:
        return "Geometry shader";
    case GL_COMPUTE_SHADER:
        return "Compute shader";
    case GL_TESS_CONTROL_SHADER:
        return "Tesselation control shader";
    case GL_TESS_EVALUATION_SHADER:
        return "Tesselation evaluation shader";
    default:
        return "Unknown shader";
    };
}
//----------------------------------------------------------------------------//
