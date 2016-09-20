#ifndef FMSTATE_H
#define FMSTATE_H

#include <QObject>
#include <QSize>
#include "debugobejct.h"

class FMState : public DebugObejct
{
    Q_OBJECT

    Q_PROPERTY(int m_viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(int m_x READ x WRITE setX)
    Q_PROPERTY(int m_y READ y WRITE setY)
    Q_PROPERTY(int m_width READ width WRITE setWidth)
    Q_PROPERTY(int m_height READ height WRITE setHeight)
    Q_PROPERTY(int m_windowState READ windowState WRITE setWindowState)
public:
    explicit FMState(QObject *parent = 0);
    ~FMState();

    int viewMode() const;
    void setViewMode(int viewMode);

    QSize size() const;
    void setSize(const QSize &size);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    int x() const;
    void setX(int x);

    int y() const;
    void setY(int y);

    int windowState() const;
    void setWindowState(int windowState);

signals:

public slots:

private:
    int m_viewMode = 1;
    int m_x = 0;
    int m_y = 0;
    int m_width = 950;
    int m_height = 600;
    int m_windowState = 0;
};


#endif // FMSTATE_H
