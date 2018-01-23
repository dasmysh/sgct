/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h
*************************************************************************/

#include <sgct/ogl_headers.h>
#include <sgct/NonLinearProjection.h>
#include <sgct/SGCTSettings.h>
#include <sgct/Engine.h>
#include <sgct/MessageHandler.h>
#include <algorithm>

sgct_core::NonLinearProjection::NonLinearProjection()
{
    mCubemapResolution = 512;
    mInterpolationMode = Linear;
    mUseDepthTransformation = false;
    mStereo = false;

    for (unsigned int & mTexture : mTextures)
        mTexture = GL_FALSE;

    mCubeMapFBO_Ptr = nullptr;
    mVBO = GL_FALSE;
    mVAO = GL_FALSE;
    mSamples = 1;

    mClearColor.r = 0.3f;
    mClearColor.g = 0.3f;
    mClearColor.b = 0.3f;
    mClearColor.a = 1.0f;

    mVpCoords[0] = 0;
    mVpCoords[1] = 0;
    mVpCoords[2] = 0;
    mVpCoords[3] = 0;

    mVerts = nullptr;

    mPreferedMonoFrustumMode = Frustum::MonoEye;
}

sgct_core::NonLinearProjection::~NonLinearProjection()
{
    for (unsigned int & mTexture : mTextures)
        if (mTexture != GL_FALSE)
        {
            glDeleteTextures(1, &mTexture);
            mTexture = GL_FALSE;
        }

    if (mVerts)
    {
        delete[] mVerts;
        mVerts = nullptr;
    }

    if (mCubeMapFBO_Ptr)
    {
        mCubeMapFBO_Ptr->destroy();
        delete mCubeMapFBO_Ptr;
    }

    if (mVBO)
    {
        glDeleteBuffers(1, &mVBO);
        mVBO = GL_FALSE;
    }

    if (mVAO)
    {
        glDeleteVertexArrays(1, &mVAO);
        mVAO = GL_FALSE;
    }

    mShader.deleteProgram();
    mDepthCorrectionShader.deleteProgram();
}

/*!
Init the non linear projection. The arguments should match the texture settings for the parent window's FBO target.
*/
void sgct_core::NonLinearProjection::init(int internalTextureFormat, unsigned int textureFormat, unsigned int textureType, int samples)
{
    mTextureInternalFormat = internalTextureFormat;
    mTextureFormat = textureFormat;
    mTextureType = textureType;
    mSamples = samples;

    initViewports();
    initTextures();
    initFBO();
    initVBO();
    initShaders();
}

void sgct_core::NonLinearProjection::updateFrustums(const sgct_core::Frustum::FrustumMode &frustumMode, const float & near_clipping_plane, const float & far_clipping_plane)
{
    for (sgct_core::BaseViewport & subViewport : mSubViewports)
        if(subViewport.isEnabled())
            subViewport.calculateNonLinearFrustum(frustumMode,
                near_clipping_plane,
                far_clipping_plane);
}

/*!
Set the resolution of the cubemap faces.

@param res the pixel resolution of each quad
*/
void sgct_core::NonLinearProjection::setCubemapResolution(int res)
{
    mCubemapResolution = res;
}

/*!
Set the resolution of the cubemap faces.

@param quality the quality hint
- low (256x256)
- medium (512x512)
- high (1024x1024)
- 1k (1024x1024)
- 2k (2048x2048)
- 4k (4096x4096)
- 8k (8192x8192)
- 16k (16384x16384)
*/
void sgct_core::NonLinearProjection::setCubemapResolution(std::string quality)
{
    int res = getCubemapRes(quality);
    if (res > 0)
        setCubemapResolution(res);
}

/*!
Set the interpolation mode

@param im the selected mode
*/
void sgct_core::NonLinearProjection::setInterpolationMode(InterpolationMode im)
{
    mInterpolationMode = im;
}

/*!
Set if depth should be re-calculated to match the non linear projection.
*/
void sgct_core::NonLinearProjection::setUseDepthTransformation(bool state)
{
    mUseDepthTransformation = state;
}

/*!
Set if stereoscopic rendering will be enabled.
*/
void sgct_core::NonLinearProjection::setStereo(bool state)
{
    mStereo = state;
}

/*!
Set the clear color (background color) for the non linear projection renderer.

@param red the red color component
@param green the green color component
@param blue the blue color component
@param alpha the alpha color component
*/
void sgct_core::NonLinearProjection::setClearColor(float red, float green, float blue, float alpha)
{
    mClearColor.r = red;
    mClearColor.g = green;
    mClearColor.b = blue;
    mClearColor.a = alpha;
}

/*!
Set the clear color (background color) for the non linear projection renderer.

@param color is the RGBA color vector
*/
void sgct_core::NonLinearProjection::setClearColor(glm::vec4 color)
{
    mClearColor = color;
}

/*!
Set the alpha clear color value for the non linear projection renderer.

@param alpha is the alpha value
*/
void sgct_core::NonLinearProjection::setAlpha(float alpha)
{
    mClearColor.a = alpha;
}

/*!
Set which projection frustum to use for mono projections (can be used for custom passive stereo)

@param fm the prefered mono frustum mode
*/
void sgct_core::NonLinearProjection::setPreferedMonoFrustumMode(sgct_core::Frustum::FrustumMode fm)
{
    mPreferedMonoFrustumMode = fm;
}

/*!
@returns the resolution of the cubemap
*/
const int & sgct_core::NonLinearProjection::getCubemapResolution() const
{
    return mCubemapResolution;
}

/*!
@returns the interpolation mode used for the non linear rendering
*/
const sgct_core::NonLinearProjection::InterpolationMode & sgct_core::NonLinearProjection::getInterpolationMode() const
{
    return mInterpolationMode;
}

void sgct_core::NonLinearProjection::initTextures()
{    
    if (sgct::Engine::instance()->getRunMode() <= sgct::Engine::OpenGL_Compablity_Profile)
    {
        glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_2D);
    }
    
    generateCubeMap(CubeMapColor, mTextureInternalFormat, mTextureFormat, mTextureType);
    if (sgct::Engine::checkForOGLErrors())
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: %dx%d color cube map texture (id: %d) generated!\n",
            mCubemapResolution, mCubemapResolution, mTextures[CubeMapColor]);
    else
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Error occured while generating %dx%d color cube texture (id: %d)!\n",
            mCubemapResolution, mCubemapResolution, mTextures[CubeMapColor]);
    
    if (sgct::SGCTSettings::instance()->useDepthTexture())
    {
        generateCubeMap(CubeMapDepth, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
        if (sgct::Engine::checkForOGLErrors())
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: %dx%d depth cube map texture (id: %d) generated!\n",
                mCubemapResolution, mCubemapResolution, mTextures[CubeMapDepth]);
        else
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Error occured while generating %dx%d depth cube texture (id: %d)!\n",
                mCubemapResolution, mCubemapResolution, mTextures[CubeMapDepth]);

        if (mUseDepthTransformation)
        {
            //generate swap textures
            generateMap(DepthSwap, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT);
            if (sgct::Engine::checkForOGLErrors())
                sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: %dx%d depth swap map texture (id: %d) generated!\n",
                    mCubemapResolution, mCubemapResolution, mTextures[DepthSwap]);
            else
                sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Error occured while generating %dx%d depth swap texture (id: %d)!\n",
                    mCubemapResolution, mCubemapResolution, mTextures[DepthSwap]);

            generateMap(ColorSwap, mTextureInternalFormat, mTextureFormat, mTextureType);
            if (sgct::Engine::checkForOGLErrors())
                sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: %dx%d color swap map texture (id: %d) generated!\n",
                    mCubemapResolution, mCubemapResolution, mTextures[ColorSwap]);
            else
                sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Error occured while generating %dx%d color swap texture (id: %d)!\n",
                    mCubemapResolution, mCubemapResolution, mTextures[ColorSwap]);
        }
    }

    if (sgct::SGCTSettings::instance()->useNormalTexture())
    {
        generateCubeMap(CubeMapNormals, sgct::SGCTSettings::instance()->getBufferFloatPrecisionAsGLint(), GL_BGR, GL_FLOAT);
        if (sgct::Engine::checkForOGLErrors())
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: %dx%d normal cube map texture (id: %d) generated!\n",
                mCubemapResolution, mCubemapResolution, mTextures[CubeMapNormals]);
        else
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Error occured while generating %dx%d normal cube texture (id: %d)!\n",
                mCubemapResolution, mCubemapResolution, mTextures[CubeMapNormals]);
    }

    if (sgct::SGCTSettings::instance()->usePositionTexture())
    {
        generateCubeMap(CubeMapPositions, sgct::SGCTSettings::instance()->getBufferFloatPrecisionAsGLint(), GL_BGR, GL_FLOAT);
        if (sgct::Engine::checkForOGLErrors())
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: %dx%d position cube map texture (id: %d) generated!\n",
                mCubemapResolution, mCubemapResolution, mTextures[CubeMapPositions]);
        else
            sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Error occured while generating %dx%d position cube texture (id: %d)!\n",
                mCubemapResolution, mCubemapResolution, mTextures[CubeMapPositions]);
    }

    if (sgct::Engine::instance()->getRunMode() <= sgct::Engine::OpenGL_Compablity_Profile)
        glPopAttrib();
}

void sgct_core::NonLinearProjection::initFBO()
{
    mCubeMapFBO_Ptr = new sgct_core::OffScreenBuffer();
    mCubeMapFBO_Ptr->setInternalColorFormat(mTextureInternalFormat);
    mCubeMapFBO_Ptr->createFBO(mCubemapResolution,
        mCubemapResolution,
        mSamples);

    if (mCubeMapFBO_Ptr->checkForErrors())
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: Cube map FBO created.\n");
    else
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Cube map FBO created with errors!\n");

    sgct_core::OffScreenBuffer::unBind();
}

void sgct_core::NonLinearProjection::initVBO()
{
    mVerts = new float[20];
    for (std::size_t i = 0; i < 20; i++)
        mVerts[i] = 0.0f;
    
    if (!sgct::Engine::instance()->isOGLPipelineFixed())
    {
        glGenVertexArrays(1, &mVAO);
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: Generating VAO: %d\n", mVAO);
    }

    glGenBuffers(1, &mVBO);
    sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: Generating VBO: %d\n", mVBO);
    
    if (!sgct::Engine::instance()->isOGLPipelineFixed())
        glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), mVerts, GL_STREAM_DRAW); //2TF + 3VF = 2*4 + 3*4 = 20
    if (!sgct::Engine::instance()->isOGLPipelineFixed())
    {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            2,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            5 * sizeof(float),    // stride
            reinterpret_cast<void*>(0) // array buffer offset
            );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            5 * sizeof(float),    // stride
            reinterpret_cast<void*>(8) // array buffer offset
            );
    }

    //unbind
    if (!sgct::Engine::instance()->isOGLPipelineFixed())
        glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void sgct_core::NonLinearProjection::generateCubeMap(TextureIndex ti, int internalFormat, unsigned int format, unsigned int type)
{
    if (mTextures[ti] != GL_FALSE)
    {
        glDeleteTextures(1, &mTextures[ti]);
        mTextures[ti] = GL_FALSE;
    }

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    GLint MaxCubeMapRes;
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &MaxCubeMapRes);
    if (mCubemapResolution > MaxCubeMapRes)
    {
        mCubemapResolution = MaxCubeMapRes;
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_DEBUG, "NonLinearProjection: Cubemap size set to max size: %d\n", MaxCubeMapRes);
    }

    //set up texture target
    glGenTextures(1, &mTextures[ti]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextures[ti]);

    //---------------------
    // Disable mipmaps
    //---------------------
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    if(sgct::Engine::instance()->isOGLPipelineFixed() || sgct::SGCTSettings::instance()->getForceGlTexImage2D())
    {
        for (int side = 0; side < 6; ++side)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, 0, internalFormat, mCubemapResolution, mCubemapResolution, 0, format, type, nullptr);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, internalFormat, mCubemapResolution, mCubemapResolution);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void sgct_core::NonLinearProjection::setupViewport(const std::size_t & face)
{
    auto cmRes = static_cast<float>(mCubemapResolution);

    //round values instead of just truncate
    mVpCoords[0] =
        static_cast<int>(mSubViewports[face].getX() * cmRes + 0.5f);
    mVpCoords[1] =
        static_cast<int>(mSubViewports[face].getY() * cmRes + 0.5f);
    mVpCoords[2] =
        static_cast<int>(mSubViewports[face].getXSize() * cmRes + 0.5f);
    mVpCoords[3] =
        static_cast<int>(mSubViewports[face].getYSize() * cmRes + 0.5f);

    glViewport(mVpCoords[0],
        mVpCoords[1],
        mVpCoords[2],
        mVpCoords[3]);

    glScissor(mVpCoords[0],
        mVpCoords[1],
        mVpCoords[2],
        mVpCoords[3]);
}

void sgct_core::NonLinearProjection::generateMap(TextureIndex ti, int internalFormat, unsigned int format, unsigned int type)
{
    if (mTextures[ti] != GL_FALSE)
    {
        glDeleteTextures(1, &mTextures[ti]);
        mTextures[ti] = GL_FALSE;
    }

    GLint MaxMapRes;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &MaxMapRes);
    if (mCubemapResolution > MaxMapRes)
    {
        sgct::MessageHandler::instance()->print(sgct::MessageHandler::NOTIFY_ERROR, "NonLinearProjection: Requested map size is too big (%d > %d)!\n", mCubemapResolution, MaxMapRes);
    }

    //set up texture target
    glGenTextures(1, &mTextures[ti]);
    glBindTexture(GL_TEXTURE_2D, mTextures[ti]);

    //---------------------
    // Disable mipmaps
    //---------------------
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    if (sgct::Engine::instance()->isOGLPipelineFixed() || sgct::SGCTSettings::instance()->getForceGlTexImage2D())
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mCubemapResolution, mCubemapResolution, 0, format, type, nullptr);
    }
    else
    {
        glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, mCubemapResolution, mCubemapResolution);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int sgct_core::NonLinearProjection::getCubemapRes(std::string& quality)
{
    std::transform(quality.begin(), quality.end(), quality.begin(), ::tolower);

    static const std::unordered_map<std::string, int> Map = {
        { "low",     256 },
        { "256",     256 },
        { "medium",  512 },
        { "512",     512 },
        { "high",   1024 },
        { "1k",     1024 },
        { "1024",   1024 },
        { "1.5k",   1536 },
        { "1536",   1536 },
        { "2k",     2048 },
        { "2048",   2048 },
        { "4k",     4096 },
        { "4096",   4096 },
        { "8k",     8192 },
        { "8192",   8192 },
        { "16k",   16384 },
        { "16384", 16384 },
    };

    auto it = Map.find(quality);
    if (it != Map.end()) {
        return it->second;
    }
    else {
        return -1;
    }
}