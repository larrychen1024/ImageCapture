#ifndef CAMERA_H
#define CAMERA_H

#include <QWidget>
#include <QDateTime>
#include <QMessageBox>
#include "imageconvert.h"
#include "common.h"
namespace Ui {
class Camera;
}

class Camera : public QWidget ,ImageConvert
{
    Q_OBJECT
public:
    explicit Camera(QWidget *parent = 0);
    ~Camera();
private:
    Ui::Camera  *ui;
    QString     *StateInfo;
    QTimer      *FrameTimer;
    //Display Device
    tPixelDatas tFbPixelData;           //Framerbuffer Pixel Data infomation
    //Video Device
    int         iFd;
    int         isCameraStart;
    int         CameraState;
    int         isCapture;
    int         iFrameCnt;
    const char  *strDevName;
    tVideoDev   tVideoDevice;           //VideoDevice infomation
    tVideoBuf   tVideoBuffer;
    tVideoBuf   tConvertBuffer;
    QImage      ImageFrame;
    //Camera information
    QString     InfoStr;
    //Photo information
    QDateTime   SysDateTime;
    QString     DateStr;
public:
    //Video Device operation
    int InitCameraDevice();
    int ExitDevice();
    int GetFrameFromCamera();
    int PutFrameToCamera();
    int StartDevice();
    int StopDevice();
    int AsynchronousIO();
    int SavePhoto();
    //GUI update
    void UpdateInfo(QString Info);
    //Camera Infomation
    void InitDisplayInfo();
public slots:
    void StartCamera();
    void QuitCamera();
    void Capture();
    void Record();
    void UpdateFrame();
    //GUI update
    void ShowDisplayInfo();
};

#endif // CAMERA_H
