#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "abstractscreenmanager.h"
#include <QVector>
#include <QMap>

class QScreen;
class ScreenObject;
class DBusDisplay;
class ScreenManager : public AbstractScreenManager
{
    Q_OBJECT
public:
    ScreenManager(QObject *parent = nullptr);
    ~ScreenManager() Q_DECL_OVERRIDE;
    ScreenPointer primaryScreen() Q_DECL_OVERRIDE;
    QVector<ScreenPointer> screens() const Q_DECL_OVERRIDE;
    QVector<ScreenPointer> logicScreens() const Q_DECL_OVERRIDE;
    ScreenPointer screen(const QString &name) const Q_DECL_OVERRIDE;
    qreal devicePixelRatio() const Q_DECL_OVERRIDE;
    DisplayMode displayMode() const Q_DECL_OVERRIDE;
    void reset() Q_DECL_OVERRIDE;
protected slots:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void onScreenGeometryChanged(const QRect &);
    void onScreenAvailableGeometryChanged(const QRect &);
    void onDockChanged();
private:
    void init();
    void connectScreen(ScreenPointer);
    void disconnectScreen(ScreenPointer);
protected:
    QMap<QScreen *,ScreenPointer> m_screens;
    DBusDisplay *m_display = nullptr;
};

#endif // SCREENMANAGER_H
