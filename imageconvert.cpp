#include "imageconvert.h"

int g_aiSupportedFormats[4] = {V4L2_PIX_FMT_UYVY,V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_RGB565};
ImageConvert::ImageConvert()
{

}

ImageConvert::~ImageConvert()
{

}

int ImageConvert::InitConvert(){
    qDebug("[Convert]:InitLut()");
    initLut();
    return 0;
}

int ImageConvert::ExitConvert(){
    qDebug("[Convert]:freeLut");
    freeLut();
    return 0;
}

int ImageConvert::isSupportYuv2Rgb(int iPixelFormatIn, int iPixelFormatOut)
{
    //qDebug("[Convert]:%d => %d",iPixelFormatIn, iPixelFormatOut);
    if (iPixelFormatIn != V4L2_PIX_FMT_YUYV){
        return -1;
    }

    if ((iPixelFormatOut != V4L2_PIX_FMT_RGB565) && (iPixelFormatOut != V4L2_PIX_FMT_RGB24) && (iPixelFormatOut != V4L2_PIX_FMT_RGB32))
    {a
        return -1;
    }
    return 0;
}

unsigned int ImageConvert::Pyuv422torgb565(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
    unsigned int i, size;
    unsigned char Y, Y1, U, V;
    unsigned char *buff = input_ptr;
    unsigned char *output_pt = output_ptr;

    unsigned int r, g, b;
    unsigned int color;

    size = image_width * image_height /2;
    for (i = size; i > 0; i--) {
        /* bgr instead rgb ?? */
        Y = buff[0] ;
        U = buff[1] ;
        Y1 = buff[2];
        V = buff[3];
        buff += 4;

        r = R_FROMYV(Y,V);
        g = G_FROMYUV(Y,U,V); //b
        b = B_FROMYU(Y,U); //v

        /* 把r,g,b三色构造为rgb565的16位值 */
        r = r >> 3;
        g = g >> 2;
        b = b >> 3;
        color = (r << 11) | (g << 5) | b;
        *output_pt++ = color & 0xff;
        *output_pt++ = (color >> 8) & 0xff;

        r = R_FROMYV(Y1,V);
        g = G_FROMYUV(Y1,U,V); //b
        b = B_FROMYU(Y1,U); //v

        /* 把r,g,b三色构造为rgb565的16位值 */
        r = r >> 3;
        g = g >> 2;
        b = b >> 3;
        color = (r << 11) | (g << 5) | b;
        *output_pt++ = color & 0xff;
        *output_pt++ = (color >> 8) & 0xff;
    }

    return 0;
}

unsigned int ImageConvert::Pyuv422torgb24(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    unsigned int in, out = 0;
     unsigned int pixel_16;
     unsigned char pixel_24[3];
     unsigned int pixel32;
     int y0, u, y1, v;

     for(in = 0; in < width * height * 2; in += 4)
     {
             pixel_16 =
                             yuv[in + 3] << 24 |
                             yuv[in + 2] << 16 |
                             yuv[in + 1] <<  8 |
                             yuv[in + 0];
             y0 = (pixel_16 & 0x000000ff);
             u  = (pixel_16 & 0x0000ff00) >>  8;
             y1 = (pixel_16 & 0x00ff0000) >> 16;
             v  = (pixel_16 & 0xff000000) >> 24;
             pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
             pixel_24[0] = (pixel32 & 0x000000ff);
             pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
             pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
             rgb[out++] = pixel_24[0];
             rgb[out++] = pixel_24[1];
             rgb[out++] = pixel_24[2];
             pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
             pixel_24[0] = (pixel32 & 0x000000ff);
             pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
             pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
             rgb[out++] = pixel_24[0];
             rgb[out++] = pixel_24[1];
             rgb[out++] = pixel_24[2];
     }
     return 0;
}

unsigned int ImageConvert::Pyuv422torgb32(unsigned char * input_ptr, unsigned char * output_ptr, unsigned int image_width, unsigned int image_height)
{
    unsigned int i, size;
    unsigned char Y, Y1, U, V;
    unsigned char *buff = input_ptr;
    unsigned int *output_pt = (unsigned int *)output_ptr;

    unsigned int r, g, b;
    unsigned int color;

    size = image_width * image_height /2;
    for (i = size; i > 0; i--) {
        /* bgr instead rgb ?? */
        Y = buff[0] ;
        U = buff[1] ;
        Y1 = buff[2];
        V = buff[3];
        buff += 4;

        r = R_FROMYV(Y,V);
        g = G_FROMYUV(Y,U,V); //b
        b = B_FROMYU(Y,U); //v
        /* rgb888 */
        color = (r << 16) | (g << 8) | b;
        *output_pt++ = color;

        r = R_FROMYV(Y1,V);
        g = G_FROMYUV(Y1,U,V); //b
        b = B_FROMYU(Y1,U); //v
        color = (r << 16) | (g << 8) | b;
        *output_pt++ = color;
    }

    return 0;
}

int ImageConvert::Yuv2RgbConvert(tVideoBuf * ptVideoBufIn, tVideoBuf * ptVideoBufOut)
{


    if (ptVideoBufOut->tFrameInfo.iFormat == V4L2_PIX_FMT_RGB32)
    {
        ptVideoBufOut->tFrameInfo.iColorDepth   = 32;
        ptVideoBufOut->tFrameInfo.iLineSize     = ptVideoBufOut->tFrameInfo.iWidth * ptVideoBufOut->tFrameInfo.iColorDepth / 8;
        ptVideoBufOut->tFrameInfo.iTotalSize    = ptVideoBufOut->tFrameInfo.iLineSize * ptVideoBufOut->tFrameInfo.iHeight;
        if (!ptVideoBufOut->aucPixelDatas) //初始化的时候，已经将其清零
        {
            ptVideoBufOut->aucPixelDatas = (unsigned char *)malloc(ptVideoBufOut->tFrameInfo.iTotalSize);
        }
        Pyuv422torgb32(ptVideoBufIn->aucPixelDatas, ptVideoBufOut->aucPixelDatas, ptVideoBufOut->tFrameInfo.iWidth, ptVideoBufOut->tFrameInfo.iHeight);
        return 0;
    }
    else if (ptVideoBufOut->tFrameInfo.iFormat== V4L2_PIX_FMT_RGB565)
    {
        ptVideoBufOut->tFrameInfo.iColorDepth   = 16;
        ptVideoBufOut->tFrameInfo.iLineSize     = ptVideoBufOut->tFrameInfo.iWidth * ptVideoBufOut->tFrameInfo.iColorDepth / 8;
        ptVideoBufOut->tFrameInfo.iTotalSize    = ptVideoBufOut->tFrameInfo.iLineSize * ptVideoBufOut->tFrameInfo.iHeight;

        if (!ptVideoBufOut->aucPixelDatas)
        {
            ptVideoBufOut->aucPixelDatas = (unsigned char *)malloc(ptVideoBufOut->tFrameInfo.iTotalSize);
        }

        Pyuv422torgb565(ptVideoBufIn->aucPixelDatas, ptVideoBufOut->aucPixelDatas, ptVideoBufOut->tFrameInfo.iWidth, ptVideoBufOut->tFrameInfo.iHeight);
        return 0;
    }
    else if (ptVideoBufOut->tFrameInfo.iFormat == V4L2_PIX_FMT_RGB24)
    {
        //qDebug("[Convert Info]:Video Output Format :V4L2_PIX_FMT_RGB24");
        ptVideoBufOut->tFrameInfo.iColorDepth   = 24;
        ptVideoBufOut->tFrameInfo.iLineSize     = ptVideoBufOut->tFrameInfo.iWidth * ptVideoBufOut->tFrameInfo.iColorDepth / 8;
        ptVideoBufOut->tFrameInfo.iTotalSize    = ptVideoBufOut->tFrameInfo.iLineSize * ptVideoBufOut->tFrameInfo.iHeight;
        //qDebug("[Convert Info]:Total Covert %d bytes", ptVideoBufOut->tFrameInfo.iTotalSize);
        if (!ptVideoBufOut->aucPixelDatas)
        {
            ptVideoBufOut->aucPixelDatas = (unsigned char *)malloc(ptVideoBufOut->tFrameInfo.iTotalSize);
        }

        Pyuv422torgb24(ptVideoBufIn->aucPixelDatas, ptVideoBufOut->aucPixelDatas, ptVideoBufOut->tFrameInfo.iWidth, ptVideoBufOut->tFrameInfo.iHeight);
        return 0;
    }

    return -1;
}

int ImageConvert::convert_yuv_to_rgb_pixel(int y, int u, int v)
{
        unsigned int pixel32 = 0;
        unsigned char *pixel = (unsigned char *)&pixel32;
        int r, g, b;
        r = y + (1.370705 * (v-128));
        g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
        b = y + (1.732446 * (u-128));
        if(r > 255) r = 255;
        if(g > 255) g = 255;
        if(b > 255) b = 255;
        if(r < 0) r = 0;
        if(g < 0) g = 0;
        if(b < 0) b = 0;
        pixel[0] = r ;
        pixel[1] = g ;
        pixel[2] = b ;
        return pixel32;
}


int ImageConvert::isSupportThisFormat(int iPixelFormat)
{
    unsigned int i;

    for (i = 0; i < sizeof(g_aiSupportedFormats)/sizeof(g_aiSupportedFormats[0]); i++)
    {
        if (g_aiSupportedFormats[i] == iPixelFormat)
            return 1;
    }
    return 0;
}
