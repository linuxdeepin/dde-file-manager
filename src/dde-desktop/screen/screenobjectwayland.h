#ifndef SCREENOBJECTWAYLAND_H
#define SCREENOBJECTWAYLAND_H

#include "abstractscreen.h"

class DBusMonitor;
class ScreenObjectWayland : public AbstractScreen
{
public:
    ScreenObjectWayland(DBusMonitor *monitor, QObject *parent = nullptr);
    ~ScreenObjectWayland() override;
    QString name() const override;
    QRect geometry() const override;
    QRect availableGeometry() const override;
    QRect handleGeometry() const override;
    QString path() const;
    bool enabled() const;
private:
    void init();
private:
    DBusMonitor *m_monitor = nullptr;
};

#endif // SCREENOBJECTWAYLAND_H
