#include "camera.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Camera CameraDevice;
    CameraDevice.show();

    return a.exec();
}
