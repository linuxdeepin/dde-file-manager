#ifndef ABSTRACTSCREENMANAGER_H
#define ABSTRACTSCREENMANAGER_H

#include "abstractscreen.h"
#include <QObject>

class AbstractScreenManager : public QObject
{
    Q_OBJECT
public:
    enum DisplayMode{Custom = 0,Duplicate,Extend,Showonly};
public:
    explicit AbstractScreenManager(QObject *parent = nullptr);
    virtual ~AbstractScreenManager();
    virtual ScreenPointer primaryScreen() = 0;
    virtual QVector<ScreenPointer> screens() const = 0;
    virtual qreal devicePixelRatio() const = 0;
    virtual DisplayMode displayMode() const = 0;
signals:
    void sigScreenChanged();
    void sigDisplayModeChanged();
};

#endif // ABSTRACTSCREENMANAGER_H
