#ifndef SCREENMANAGERWAYLAND_H
#define SCREENMANAGERWAYLAND_H

#include "abstractscreenmanager.h"
#include <QVector>
#include <QMap>

class DBusDisplay;
class ScreenManagerWayland : public AbstractScreenManager
{
    Q_OBJECT
public:
    ScreenManagerWayland(QObject *parent = nullptr);
    ~ScreenManagerWayland() Q_DECL_OVERRIDE;
    ScreenPointer primaryScreen() Q_DECL_OVERRIDE;
    QVector<ScreenPointer> screens() const Q_DECL_OVERRIDE;
    QVector<ScreenPointer> logicScreens() const Q_DECL_OVERRIDE;
    ScreenPointer screen(const QString &name) const Q_DECL_OVERRIDE;
    qreal devicePixelRatio() const Q_DECL_OVERRIDE;
    DisplayMode displayMode() const Q_DECL_OVERRIDE;
    void reset() Q_DECL_OVERRIDE;
protected slots:
    void onMonitorChanged();
    void onDockChanged();
    void onScreenGeometryChanged(const QRect &rect);
private:
    void init();
    void connectScreen(ScreenPointer);
    void disconnectScreen(ScreenPointer);
protected:
    QMap<QString,ScreenPointer> m_screens; //dbus-path - screen
    DBusDisplay *m_display = nullptr;
};

#endif // SCREENMANAGERWAYLAND_H
