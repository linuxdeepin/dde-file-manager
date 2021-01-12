#ifndef ABSTRACTSCREENMANAGER_H
#define ABSTRACTSCREENMANAGER_H

#include "abstractscreen.h"
#include <QObject>
#include <QRect>

class ScreenManagerPrivate;
class AbstractScreenManager : public QObject
{
    Q_OBJECT
    friend class ScreenManagerPrivate;
public:
    enum DisplayMode{Custom = 0,Duplicate,Extend,Showonly}; //显示模式
    enum Event{Screen,Mode,Geometry,AvailableGeometry}; //事件类型
public:
    explicit AbstractScreenManager(QObject *parent = nullptr);
    virtual ~AbstractScreenManager();
    virtual ScreenPointer primaryScreen() = 0;
    virtual QVector<ScreenPointer> screens() const = 0; //按接入顺序
    virtual QVector<ScreenPointer> logicScreens() const = 0;    //主屏第一，其他按接入顺序
    virtual ScreenPointer screen(const QString &name) const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual DisplayMode displayMode() const = 0;        //从dbus获取后台最新mode
    virtual DisplayMode lastChangedMode() const = 0;    //最后接收到改变后的mode
    virtual void reset() = 0;
protected:
    void appendEvent(Event);    //添加屏幕事件
signals:
    void sigScreenChanged();    //屏幕接入，移除
    void sigDisplayModeChanged();   //显示模式改变
    void sigScreenGeometryChanged(); //屏幕分辨率改变
    void sigScreenAvailableGeometryChanged(); //屏幕可用区改变
protected:
#ifndef UNUSE_TEMP //临时方案，判断主屏大小改变时，是否是合并/拆分
    int m_lastMode = -1;
#endif
private:
    ScreenManagerPrivate *d;
};

#endif // ABSTRACTSCREENMANAGER_H
