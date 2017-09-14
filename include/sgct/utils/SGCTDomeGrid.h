/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _SGCT_DOME_GRID
#define _SGCT_DOME_GRID

namespace sgct_utils
{

/*!
    Helper class to render a dome grid
*/
class SGCTDomeGrid
{
public:
    SGCTDomeGrid(float radius, float FOV, unsigned int segments, unsigned int rings, unsigned int resolution = 128);

    SGCTDomeGrid(const SGCTDomeGrid & dome) = delete;
    const SGCTDomeGrid & operator=(const SGCTDomeGrid & dome) = delete;

    ~SGCTDomeGrid();
    void draw();

private:
    void init(float radius, float FOV, unsigned int segments, unsigned int rings, unsigned int resolution);

    void drawVBO();
    void drawVAO();

    using InternalCallbackFn = void (SGCTDomeGrid::*)();
    InternalCallbackFn    mInternalDrawFn;

    void createVBO();
    void cleanup();

private:
    float * mVerts;
    unsigned int mNumberOfVertices;
    unsigned int mResolution;
    unsigned int mRings;
    unsigned int mSegments;
    unsigned int mVBO;
    unsigned int mVAO;
};

}

#endif
