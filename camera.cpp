#include "camera.h"
#include "ui_camera.h"

Camera::Camera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Camera)
{
    ui->setupUi(this);
    StateInfo   = new QString;
    FrameTimer  = new QTimer(this);

    memset(&tVideoBuffer, 0, sizeof(tVideoBuf));
    memset(&tConvertBuffer, 0, sizeof(tVideoBuf));
    iFrameCnt       = 0;
    CameraState     = STATE_NULL;
    isCameraStart   = STOP;

    tFbPixelData.iWidth         = ui->ImageLabel->width();
    tFbPixelData.iHeight        = ui->ImageLabel->height();
    tFbPixelData.iColorDepth    = ui->ImageLabel->depth();
    tFbPixelData.iTotalSize     = tFbPixelData.iWidth * tFbPixelData.iHeight * tFbPixelData.iColorDepth / 8 ;
    tFbPixelData.iFormat        =(tFbPixelData.iColorDepth == 16) ? \
                V4L2_PIX_FMT_RGB565 : (tFbPixelData.iColorDepth == 24) ?\
                    V4L2_PIX_FMT_RGB24 : (tFbPixelData.iColorDepth == 32) ?  \
                        V4L2_PIX_FMT_RGB32 : 0;

    connect(ui->StartBtn, SIGNAL(clicked(bool)), this ,SLOT(StartCamera()));
    connect(ui->SnapBtn, SIGNAL(clicked(bool)), this ,SLOT(Capture()));
    connect(ui->RecordBtn, SIGNAL(clicked(bool)), this ,SLOT(Record()));
    connect(ui->QuitBtn, SIGNAL(clicked(bool)), this ,SLOT(QuitCamera()));

    connect(ui->StartBtn, SIGNAL(clicked(bool)), this, SLOT(ShowDisplayInfo()));
    connect(ui->SnapBtn, SIGNAL(clicked(bool)), this, SLOT(ShowDisplayInfo()));
    connect(ui->RecordBtn, SIGNAL(clicked(bool)), this, SLOT(ShowDisplayInfo()));

    connect(FrameTimer, SIGNAL(timeout()), this, SLOT(UpdateFrame()));

    strDevName = VIDEO_DEV_NAME;    // "/dev/video15"

    InitDisplayInfo();
    InitConvert();
    InitCameraDevice();

}

Camera::~Camera()
{
    delete ui;
    delete StateInfo;
    delete FrameTimer;
}

void Camera::InitDisplayInfo(){
    InfoStr = "Display Info:\n";
    InfoStr.append("Width : ");
    InfoStr.append(QString::number(tFbPixelData.iWidth, 10));
    InfoStr.append("\nHeight : ");
    InfoStr.append(QString::number(tFbPixelData.iHeight, 10));
    InfoStr.append("\nDepth : ");
    InfoStr.append(QString::number(tFbPixelData.iColorDepth, 10));
    InfoStr.append("\nSize:");
    InfoStr.append(QString::number(tFbPixelData.iTotalSize, 10));
    InfoStr.append("\nFormat:");
    InfoStr.append((tFbPixelData.iColorDepth == 16) ? "RGB565" : (tFbPixelData.iColorDepth == 24) ? "RGB24": (tFbPixelData.iColorDepth == 32) ?  "RGB32" : 0);

    UpdateInfo(InfoStr);
    ShowDisplayInfo();
}

void Camera::StartCamera(){
    if(isCameraStart == STOP){
        isCameraStart = START;
        InfoStr = "Display Info:\n";
        InfoStr.append("Width   : ");
        InfoStr.append(QString::number(tFbPixelData.iWidth, 10));
        InfoStr.append("\nHeight: ");
        InfoStr.append(QString::number(tFbPixelData.iHeight, 10));
        InfoStr.append("\nDepth : ");
        InfoStr.append(QString::number(tFbPixelData.iColorDepth, 10));

        UpdateInfo(InfoStr);
        StartDevice();
        FrameTimer->start(30);
    }
    else if(isCameraStart == START){
        QMessageBox::information(this, tr("Information"),tr("Camera has already opened!"));
    }
}

void Camera::QuitCamera(){
    if(isCameraStart == START){
        isCameraStart = STOP;
        StopDevice();
        ExitDevice();
        close();
    }
    else if(isCameraStart == STOP){
        QMessageBox::information(this, tr("Information"),tr("Camera has never opened!Please open the camera first!"));
    }

}

void Camera::Capture(){
    InfoStr = "Capture Info:\n";
    InfoStr.append("Width : ");
    InfoStr.append(QString::number(tFbPixelData.iWidth, 10));
    InfoStr.append("\nHeight : ");
    InfoStr.append(QString::number(tFbPixelData.iHeight, 10));
    InfoStr.append("\nDepth : ");
    InfoStr.append(QString::number(tFbPixelData.iColorDepth, 10));

    UpdateInfo(InfoStr);
    UpdateInfo("Snap");

    CameraState = CAPTURE;
}

void Camera::Record(){
    InfoStr = "Record Info:\n";
    InfoStr.append("Width : ");
    InfoStr.append(QString::number(tFbPixelData.iWidth, 10));
    InfoStr.append("\nHeight : ");
    InfoStr.append(QString::number(tFbPixelData.iHeight, 10));
    InfoStr.append("\nDepth : ");
    InfoStr.append(QString::number(tFbPixelData.iColorDepth, 10));
    UpdateInfo(InfoStr);
}

void Camera::UpdateFrame(){
    int iError;

    InfoStr="\nFrame ";
    InfoStr.append(QString::number(++iFrameCnt, 10));
    UpdateInfo(InfoStr);

    iError = GetFrameFromCamera();
    if (iError)
    {
        qDebug("GetFrame for %s error!", strDevName);
        return ;
    }

    if (tVideoBuffer.tFrameInfo.iFormat != tFbPixelData.iFormat)
    {
        iError = isSupportYuv2Rgb(tVideoBuffer.tFrameInfo.iFormat, tFbPixelData.iFormat);
        if(iError)
        {
            qDebug("[Camera]:Can't convert this format!");
        }

        tConvertBuffer.tFrameInfo.iFormat   = tFbPixelData.iFormat;
        tConvertBuffer.tFrameInfo.iHeight   = tFbPixelData.iHeight;
        tConvertBuffer.tFrameInfo.iWidth    = tFbPixelData.iWidth;
        iError = Yuv2RgbConvert(&tVideoBuffer, &tConvertBuffer);
        if(iError){
            qDebug("[Camera]:Convert for %s error!", VIDEO_DEV_NAME);
            return;
        }
    }

    ImageFrame = QImage((const uchar*)tConvertBuffer.aucPixelDatas, tConvertBuffer.tFrameInfo.iWidth, tConvertBuffer.tFrameInfo.iHeight, QImage::Format_RGB32);
    ui->ImageLabel->setPixmap(QPixmap::fromImage(ImageFrame));

    if(CameraState == CAPTURE){
        CameraState = STATE_NULL;
        SavePhoto();
    }

    PutFrameToCamera();
}


void Camera::ShowDisplayInfo(){
    ui->InfoLabel->setText(*StateInfo);
}

void Camera::UpdateInfo(QString Info){
    *StateInfo = Info;
}


int Camera::InitCameraDevice(){
    int i;

    int iError;
    struct v4l2_capability      tV4l2Cap;
    struct v4l2_fmtdesc         tFmtDesc;
    struct v4l2_format          tV4l2Fmt;
    struct v4l2_requestbuffers  tV4l2ReqBuffs;
    struct v4l2_buffer          tV4l2Buf;

    iFd = open(strDevName, O_RDWR | O_NONBLOCK); /*非阻塞方式打开设备文件*/
    qDebug("[Camera]:Open %s, iFd = %d", strDevName, iFd);
    if (iFd < 0)
    {
        qDebug("can not open %s\n", strDevName);
        return -1;
    }

    iError = ioctl(iFd, VIDIOC_QUERYCAP, &tV4l2Cap);
    memset(&tV4l2Cap, 0, sizeof(struct v4l2_capability));
    iError = ioctl(iFd, VIDIOC_QUERYCAP, &tV4l2Cap);
    if (iError) {
        qDebug("Error opening device %s: unable to query device.", strDevName);
        goto err_exit;
    }

    if (!(tV4l2Cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        qDebug("%s is not a video capture device", strDevName);
        goto err_exit;
    }

    if (tV4l2Cap.capabilities & V4L2_CAP_STREAMING) {
        qDebug("%s supports streaming i/o", strDevName);
    }

    if (tV4l2Cap.capabilities & V4L2_CAP_READWRITE) {
        qDebug("%s supports read i/o", strDevName);
    }

    memset(&tFmtDesc, 0, sizeof(tFmtDesc));
    tFmtDesc.index = 0;
    tFmtDesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while ((iError = ioctl(iFd, VIDIOC_ENUM_FMT, &tFmtDesc)) == 0) {
        if (Camera::isSupportThisFormat(tFmtDesc.pixelformat))
        {
            qDebug("[Camera]:support format:%s", (tFmtDesc.pixelformat == V4L2_PIX_FMT_YUYV ? "V4L2_PIX_FMT_YUYV":"FROMAT ERROR"));
            tVideoDevice.tFrameInfo.iFormat = tFmtDesc.pixelformat;
            break;
        }
        tFmtDesc.index++;
    }

    if (!tVideoDevice.tFrameInfo.iFormat)
    {
        qDebug("can not support the format of this device\n");
        goto err_exit;
    }

    /* set the input format */
    memset(&tV4l2Fmt, 0, sizeof(struct v4l2_format));
    tV4l2Fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Fmt.fmt.pix.pixelformat = tVideoDevice.tFrameInfo.iFormat;
    tV4l2Fmt.fmt.pix.width       = tFbPixelData.iWidth;
    tV4l2Fmt.fmt.pix.height      = tFbPixelData.iHeight;
    //qDebug("[Camera]:Width:%d\tHeight:%d",tFbPixelData.iWidth, tFbPixelData.iHeight);
    tV4l2Fmt.fmt.pix.field       = V4L2_FIELD_ANY;

    /* 如果驱动程序发现无法设置某些参数(比如分辨率),
     * 它会调整这些参数, 并且返回给应用程序
     */
    iError = ioctl(iFd, VIDIOC_S_FMT, &tV4l2Fmt);
    if (iError)
    {
        qDebug("Unable to set format\n");
        goto err_exit;
    }
    tVideoDevice.tFrameInfo.iWidth  = tV4l2Fmt.fmt.pix.width;
    tVideoDevice.tFrameInfo.iHeight = tV4l2Fmt.fmt.pix.height;
    //qDebug("[Camera]:Width:%d\tHeight:%d",tVideoDevice.tFrameInfo.iWidth, tVideoDevice.tFrameInfo.iHeight);

    /* request buffers */
    memset(&tV4l2ReqBuffs, 0, sizeof(struct v4l2_requestbuffers));
    tV4l2ReqBuffs.count     = BUFFER_NUM;
    tV4l2ReqBuffs.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2ReqBuffs.memory    = V4L2_MEMORY_MMAP;
    iError = ioctl(iFd, VIDIOC_REQBUFS, &tV4l2ReqBuffs);
    if (iError)
    {
        qDebug("Unable to allocate buffers.\n");
        goto err_exit;
    }

    tVideoDevice.iVideoBufCnt = tV4l2ReqBuffs.count;
    if (tV4l2Cap.capabilities & V4L2_CAP_STREAMING)
    {
        /* map the buffers */
        for (i = 0; i < tVideoDevice.iVideoBufCnt; i++)
        {
            memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
            tV4l2Buf.index  = i;
            tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            tV4l2Buf.memory = V4L2_MEMORY_MMAP;
            iError = ioctl(iFd, VIDIOC_QUERYBUF, &tV4l2Buf);
            if (iError)
            {
                qDebug("Unable to query buffer.\n");
                goto err_exit;
            }

            tVideoDevice.iVideoBufMaxLen    = tV4l2Buf.length;
            tVideoDevice.pucVideBuf[i]      = (unsigned char *)mmap(0, tV4l2Buf.length, PROT_READ, MAP_SHARED, iFd, tV4l2Buf.m.offset);
            if (tVideoDevice.pucVideBuf[i] == MAP_FAILED)
            {
                qDebug("Unable to map buffer\n");
                goto err_exit;
            }
        }

        /* Queue the buffers. */
        for (i = 0; i < tVideoDevice.iVideoBufCnt; i++)
        {
            memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
            tV4l2Buf.index  = i;
            tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            tV4l2Buf.memory = V4L2_MEMORY_MMAP;
            iError = ioctl(iFd, VIDIOC_QBUF, &tV4l2Buf);
            if (iError==-1)
            {
                qDebug("Unable to queue buffer.\n");
                goto err_exit;
            }
        }

    }

    return 0;
err_exit:
    ::close(iFd);
    return -1;
}

int Camera::GetFrameFromCamera(){

    int iError;
    struct v4l2_buffer tV4l2Buf;
#if 1
    struct pollfd tFds[1];
    /* poll */
    tFds[0].fd     = iFd;
    tFds[0].events = POLLIN;

    iError = poll(tFds, 1, -1);

    if (iError <= 0)
    {
        qDebug("poll error!\n");
        return -1;
    }
#endif

    /* VIDIOC_DQBUF */
    memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
    tV4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;
    iError = ioctl(iFd, VIDIOC_DQBUF, &tV4l2Buf);/* 从视频缓冲区的输出队列中取得一个已经保存有一帧视频数据的视频缓冲区*/
    if (iError < 0)
    {
        qDebug("iFd = %d\tUnable to dequeue buffer.",iFd);
        return -1;
    }
    tVideoDevice.iVideoBufCurIndex = tV4l2Buf.index;

    tVideoBuffer.tFrameInfo.iFormat     = tVideoDevice.tFrameInfo.iFormat;
    tVideoBuffer.tFrameInfo.iWidth      = tVideoDevice.tFrameInfo.iWidth;
    tVideoBuffer.tFrameInfo.iHeight     = tVideoDevice.tFrameInfo.iHeight;
    tVideoBuffer.tFrameInfo.iColorDepth = (tVideoDevice.tFrameInfo.iFormat == V4L2_PIX_FMT_YUYV) ? \
                16 : (tVideoDevice.tFrameInfo.iFormat == V4L2_PIX_FMT_MJPEG) ? \
                    0 : (tVideoDevice.tFrameInfo.iFormat == V4L2_PIX_FMT_RGB565) ? \
                        16 :  0;
    tVideoBuffer.tFrameInfo.iLineSize   = tVideoDevice.tFrameInfo.iWidth * tVideoDevice.tFrameInfo.iColorDepth / 8;
    tVideoBuffer.tFrameInfo.iTotalSize  = tV4l2Buf.bytesused;
    tVideoBuffer.aucPixelDatas          = tVideoDevice.pucVideBuf[tV4l2Buf.index];
    //qDebug("tVideoBuffer.aucPixelDatas = 0x%x", tVideoBuffer.aucPixelDatas);
    return 0;
}

int Camera::PutFrameToCamera(){
    /* VIDIOC_QBUF */
    struct v4l2_buffer tV4l2Buf;
    int iError;

    memset(&tV4l2Buf, 0, sizeof(struct v4l2_buffer));
    tV4l2Buf.index  = tVideoDevice.iVideoBufCurIndex;/*指令(指定)要投放到视频输入队列中的内核空间视频缓冲区的编号*/
    tV4l2Buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4l2Buf.memory = V4L2_MEMORY_MMAP;
    /*视频缓冲区进入视频输入队列，在启动视频设备拍摄图像时，相应的视频数据被保存到视频输入队列相应的视频缓冲区中*/

    if (iError)
    {
        qDebug("Unable to queue buffer.\n");
        return -1;
    }
    return 0;
}

int Camera::StartDevice()
{
    int iType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iError;
    /* 启动视频采集命令，视频设备驱动程序开始采集视频数据，并把采集到的视频数据保存到视频驱动的视频缓冲区中。 */
    iError = ioctl(iFd, VIDIOC_STREAMON, &iType);
    qDebug("[Camera]:Start :iFd = %d",iFd);
    if (iError)
    {
        qDebug("Unable to start capture.");
        return -1;
    }
    return 0;
}

int Camera::StopDevice()
{
    int iType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int iError;

    iError = ioctl(iFd, VIDIOC_STREAMOFF, &iType);
    if (iError)
    {
        qDebug("Unable to stop capture.\n");
        return -1;
    }
    return 0;
}

int Camera::ExitDevice(){
    int i;
    for (i = 0; i < tVideoDevice.iVideoBufCnt; i++)
    {
        if (tVideoDevice.pucVideBuf[i])
        {
            munmap(tVideoDevice.pucVideBuf[i], tVideoDevice.iVideoBufMaxLen);
            tVideoDevice.pucVideBuf[i] = NULL;
        }
    }

    ::close(iFd);
    return 0;
}

int Camera::AsynchronousIO(){
    for (;;)                    //异步IO
    {
        fd_set fds;
        struct timeval tv;
        int r;
        FD_ZERO(&fds);          //将指定的file描述符集清空
        FD_SET(iFd, &fds);      //在文件描述符集合中增1个新的文件描述符
        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        r = select(iFd + 1, &fds, NULL, NULL, &tv); //判断是否可读（即摄像头是否准备好），tv是定时
        if (-1 == r)
        {
            if (EINTR == errno)
                continue;
            qDebug("select err\n");

        }
        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }
    }
}

int Camera::SavePhoto()
{
    int iError;
    QString PhotoName;
    SysDateTime = QDateTime::currentDateTime();
    DateStr     = SysDateTime.toString("yyyyMMddhhmmss");
    PhotoName   = QCoreApplication::applicationDirPath() + "//"+DateStr + ".png";
    //qDebug("System Time:%s",DateStr.toStdString().data());
    const QPixmap * p = ui->ImageLabel->pixmap();
    iError = p->save(PhotoName, "png");
    if(iError == true){
        InfoStr = "Saved photos\n";
        //InfoStr.append("File name:" + PhotoName);
        InfoStr.append(PhotoName);
        UpdateInfo(InfoStr);
        ShowDisplayInfo();
        return 0;
    }else{
        QMessageBox::information(this, tr("Information"),tr("Save Failed!"));
        return -1;
    }

    return -1;
}
