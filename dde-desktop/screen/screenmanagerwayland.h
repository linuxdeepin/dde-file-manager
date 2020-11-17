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
    explicit ScreenManagerWayland(QObject *parent = nullptr);
    ~ScreenManagerWayland() override;
    ScreenPointer primaryScreen() override;
    QVector<ScreenPointer> screens() const override;
    QVector<ScreenPointer> logicScreens() const override;
    ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    DisplayMode displayMode() const override;
    DisplayMode lastChangedMode() const override;
    void reset() override;
protected slots:
    void onMonitorChanged();
    void onDockChanged();
    void onScreenGeometryChanged(const QRect &rect);
private:
    void init();
    void connectScreen(ScreenPointer);
    void disconnectScreen(ScreenPointer);
protected:
    QMap<QString, ScreenPointer> m_screens; //dbus-path - screen
    DBusDisplay *m_display = nullptr;
};

#endif // SCREENMANAGERWAYLAND_H
