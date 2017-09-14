/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _SGCT_BOX
#define _SGCT_BOX

#include "../helpers/SGCTVertexData.h"

/*! \namespace sgct_utils
\brief SGCT utils namespace contains basic utilities for geometry rendering
*/
namespace sgct_utils
{

/*!
    This class creates and renders a textured box.
*/
class SGCTBox
{
public:
    enum TextureMappingMode { Regular = 0, CubeMap, SkyBox };

    SGCTBox(float size, TextureMappingMode tmm = Regular);

    SGCTBox(const SGCTBox & box) = delete;
    const SGCTBox & operator=(const SGCTBox & box) = delete;

    ~SGCTBox();
    void draw();

private:
    void drawVBO();
    void drawVAO();

    using InternalCallbackFn = void (SGCTBox::*)();
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
