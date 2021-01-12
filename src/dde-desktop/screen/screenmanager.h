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
    explicit ScreenManager(QObject *parent = nullptr);
    ~ScreenManager() override;
    ScreenPointer primaryScreen() override;
    QVector<ScreenPointer> screens() const override;
    QVector<ScreenPointer> logicScreens() const override;
    ScreenPointer screen(const QString &name) const override;
    qreal devicePixelRatio() const override;
    DisplayMode displayMode() const override;
    DisplayMode lastChangedMode() const override;
    void reset() override;
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
    QMap<QScreen *, ScreenPointer> m_screens;
    DBusDisplay *m_display = nullptr;
};

#endif // SCREENMANAGER_H
