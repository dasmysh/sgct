/*************************************************************************
Copyright (c) 2012-2015 Miroslav Andel
All rights reserved.

For conditions of distribution and use, see copyright notice in sgct.h 
*************************************************************************/

#ifndef _IMAGE_H_
#define _IMAGE_H_

#ifndef SGCT_DONT_USE_EXTERNAL
#include "external/pngconf.h"
#else
#include <pngconf.h>
#endif

#include <string>

namespace sgct_core
{

class Image
{
public:
    enum ChannelType { Blue = 0, Green, Red, Alpha };
    enum FormatType { FORMAT_PNG = 0, FORMAT_JPEG, FORMAT_TGA, UNKNOWN_FORMAT };
    
    Image();
    ~Image();

    bool allocateOrResizeData();
    bool load(std::string filename);
    bool loadPNG(std::string filename);
    bool loadPNG(unsigned char * data, std::size_t len);
    bool loadJPEG(std::string filename);
    bool loadJPEG(unsigned char * data, std::size_t len);
    bool loadTGA(std::string filename);
    bool loadTGA(unsigned char * data, std::size_t len);
    bool save();
    bool savePNG(std::string filename, int compressionLevel = -1);
    bool savePNG(int compressionLevel = -1);
    bool saveJPEG(int quality = 100);
    bool saveTGA();
    void setFilename(std::string filename);
    void setPreferBGRExport(bool state);
    void setPreferBGRImport(bool state);
    bool getPreferBGRExport() const;
    bool getPreferBGRImport() const;

    unsigned char * getData();
    unsigned char * getDataAt(std::size_t x, std::size_t y);
    std::size_t getChannels() const;
    std::size_t getWidth() const;
    std::size_t getHeight() const;
    std::size_t getDataSize() const;
    std::size_t getBytesPerChannel() const;

    unsigned char * getSampleAt(std::size_t x, std::size_t y);
    void setSampleAt(unsigned char * val, std::size_t x, std::size_t y);

    //only valid for 8-bit images
    unsigned char getSampleAt(std::size_t x, std::size_t y, ChannelType c);
    void setSampleAt(unsigned char val, std::size_t x, std::size_t y, ChannelType c);
    float getInterpolatedSampleAt(float x, float y, ChannelType c);
    

    void setDataPtr(unsigned char * dPtr);
    void setSize(std::size_t width, std::size_t height);
    void setChannels(std::size_t channels);
    void setBytesPerChannel(std::size_t bpc);
    inline const char * getFilename() { return mFilename.c_str(); }

private:
    void cleanup();
    bool allocateRowPtrs();
    FormatType getFormatType(const std::string & filename);
    bool isTGAPackageRLE(unsigned char * row, std::size_t pos);
    bool decodeTGARLE(FILE * fp);
    bool decodeTGARLE(unsigned char * data, std::size_t len);
    std::size_t getTGAPackageLength(unsigned char * row, std::size_t pos, bool rle);
    
private:
    bool mExternalData;
    std::size_t mChannels;
    std::size_t mSize_x;
    std::size_t mSize_y;
    std::size_t mDataSize;
    std::size_t mBytesPerChannel;
    std::string mFilename;
    unsigned char * mData;
    png_bytep * mRowPtrs;
    bool mPreferBGRForExport;
    bool mPreferBGRForImport;
};

}

#endif

