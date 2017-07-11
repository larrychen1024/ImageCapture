#ifndef DISPLAYAREA_H
#define DISPLAYAREA_H

#include <QObject>

typedef struct tag_DisplayInfo{
    int iWidth;
    int iHeight;
    int iColorDepth;
    int iPixelSize;
}tDisplayInfo;

class DisplayArea:public QObject
{
    Q_OBJECT
public:
    explicit DisplayArea(QObject * parent = 0);
    ~DisplayArea();
    int GetWidth();
    int GetHeigt();
    int GetColorDepth();
    QString info();
signals:

public slots:

private:
    tDisplayInfo tDisplayArea;
};

#endif // DISPLAYAREA_H
