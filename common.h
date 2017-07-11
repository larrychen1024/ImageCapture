#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDebug>
#include <QTimer>

extern "C"{
#include <linux/videodev2.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <poll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <fcntl.h>      /*文件控制定义*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <asm/types.h>

#include "color.h"
}

#define BUFFER_NUM 5

typedef struct tag_PixelData{
    int     iWidth;                     /* 视频图像显示区域的宽度 				*/
    int     iHeight;					/* 视频图像显示区域的高度 				*/
    int     iColorDepth;				/* 视频图像显示区域的颜色深度 			*/
    int     iPixelSize;					/* 视频图像显示区域的像素点的字节大小 	*/
    int     iLineSize;					/* 视频图像显示区域的一行数据的字节大小	*/
    int     iTotalSize;					/* 视频图像显示区域的总共的字节大小 	*/
    int     iFormat;					/* 视频图像数据的格式 					*/
}tPixelDatas;

typedef struct tag_VideoDev{
    tPixelDatas     tFrameInfo;					/* 一帧图像数据的信息 		*/
    int             iVideoBufCnt;				/* 视频图像帧的数量 		*/
    int             iVideoBufMaxLen;			/* 视频图像帧的缓冲区长度	*/
    int             iVideoBufCurIndex;			/* 视频图像数据的格式 		*/
    unsigned char   *pucVideBuf[BUFFER_NUM]; 	/* 指针数组 				*/
}tVideoDev;

typedef struct tag_VideoBuf{
    tPixelDatas     tFrameInfo;					/* 一帧图像数据的信息 		*/
    unsigned char   *aucPixelDatas;  			/* 象素数据存储的指针 		*/
}tVideoBuf;

extern int g_aiSupportedFormats[4];

#define VIDEO_DEV_NAME "/dev/video15"

#define START       0x01
#define STOP        0x02

#define STATE_NULL  0x00
#define CAPTURE     0x01
#define RECORD      0x02
#endif // COMMON_H
