#ifndef ABSTRACTSCREENMANAGER_H
#define ABSTRACTSCREENMANAGER_H

#include "abstractscreen.h"
#include <QObject>
#include <QRect>

class AbstractScreenManager : public QObject
{
    Q_OBJECT
public:
    enum DisplayMode{Custom = 0,Duplicate,Extend,Showonly};
public:
    explicit AbstractScreenManager(QObject *parent = nullptr);
    virtual ~AbstractScreenManager();
    virtual ScreenPointer primaryScreen() = 0;
    virtual QVector<ScreenPointer> screens() const = 0; //按接入顺序
    virtual QVector<ScreenPointer> logicScreens() const = 0;    //主屏第一，其他按接入顺序
    virtual ScreenPointer screen(const QString &name) const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual DisplayMode displayMode() const = 0;
    virtual void reset() = 0;
signals:
    void sigScreenChanged();    //屏幕接入，移除
    void sigDisplayModeChanged();   //显示模式改变
    void sigScreenGeometryChanged(ScreenPointer,QRect); //屏幕分辨率改变
    void sigScreenAvailableGeometryChanged(ScreenPointer,QRect); //屏幕可用区改变
};

#endif // ABSTRACTSCREENMANAGER_H
