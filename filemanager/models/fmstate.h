#ifndef FMSTATE_H
#define FMSTATE_H

#include <QObject>
#include <QSize>
#include "utils/debugobejct.h"

class FMState : public DebugObejct
{
    Q_OBJECT

    Q_PROPERTY(int m_viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(int m_width READ width WRITE setWidth)
    Q_PROPERTY(int m_height READ height WRITE setHeight)
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

signals:

public slots:

private:
    int m_viewMode = 1;
    int m_width = 950;
    int m_height = 600;
};


#endif // FMSTATE_H
