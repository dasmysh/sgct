/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _SGCT_PLANE
#define _SGCT_PLANE

#include "../helpers/SGCTVertexData.h"

/*! \namespace sgct_utils
\brief SGCT utils namespace contains basic utilities for geometry rendering
*/
namespace sgct_utils
{

/*!
    This class creates and renders a textured box.
*/
class SGCTPlane
{
public:
    SGCTPlane(float width, float height);

    SGCTPlane(const SGCTPlane & box) = delete;
    const SGCTPlane & operator=(const SGCTPlane & box) = delete;

    ~SGCTPlane();
    void draw();

private:
    void drawVBO();
    void drawVAO();

    using InternalCallbackFn = void (SGCTPlane::*)();
    InternalCallbackFn    mInternalDrawFn;

    void cleanUp();
    void createVBO();

private:    
    unsigned int mVBO;
    unsigned int mVAO;
    sgct_helpers::SGCTVertexData * mVerts;
};
}

#endif
