#ifndef SCREENOBJECTWAYLAND_H
#define SCREENOBJECTWAYLAND_H

#include "abstractscreen.h"

class DBusMonitor;
class ScreenObjectWayland : public AbstractScreen
{
public:
    ScreenObjectWayland(DBusMonitor *monitor,QObject *parent = nullptr);
    ~ScreenObjectWayland() Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    QRect geometry() const Q_DECL_OVERRIDE;
    QRect availableGeometry() const Q_DECL_OVERRIDE;
    QRect handleGeometry() const Q_DECL_OVERRIDE;
    QString path() const;
    bool enabled() const;
private:
    void init();
private:
    DBusMonitor *m_monitor = nullptr;
};

#endif // SCREENOBJECTWAYLAND_H
