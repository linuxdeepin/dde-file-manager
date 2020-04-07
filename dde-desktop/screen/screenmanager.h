#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "abstractscreenmanager.h"
#include <QVector>

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
    qreal devicePixelRatio() const Q_DECL_OVERRIDE;
    DisplayMode displayMode() const Q_DECL_OVERRIDE;
protected slots:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
private:
    void init();
protected:
    QVector<ScreenPointer> m_screens;
    DBusDisplay *m_display = nullptr;
};

#endif // SCREENMANAGER_H
