#ifndef IMAGECONVERT_H
#define IMAGECONVERT_H
#include "common.h"
extern "C"{
    #include "color.h"
}

class ImageConvert
{
public:
    ImageConvert();
    ~ImageConvert();
    int isSupportYuv2Rgb(int iPixelFormatIn, int iPixelFormatOut);
    int InitConvert();
    int ExitConvert();
    unsigned int Pyuv422torgb565(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height);
    unsigned int Pyuv422torgb24(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    unsigned int Pyuv422torgb32(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height);
    int Yuv2RgbConvert(tVideoBuf * ptVideoBufIn, tVideoBuf * ptVideoBufOut);
    int convert_yuv_to_rgb_pixel(int y, int u, int v);
    int isSupportThisFormat(int iPixelFormat);
};

#endif // IMAGECONVERT_H
