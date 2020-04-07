#ifndef BACKGROUNDMANAGER_H
#define BACKGROUNDMANAGER_H

#include "backgroundwidget.h"
#include "screen/abstractscreen.h"

#include <com_deepin_wm.h>
#include <DWindowManagerHelper>
#include <dgiosettings.h>

#include <QObject>
#include <QMap>

using WMInter = com::deepin::wm;

DGUI_USE_NAMESPACE
class AbstractScreen;
class BackgroundManager : public QObject
{
    Q_OBJECT
public:
    explicit BackgroundManager(bool preview = false,QObject *parent = nullptr);
    ~BackgroundManager();
    bool isEnabled() const;
signals:
    void sigBackgroundEnableChanged();
public slots:
    void onBackgroundBuild();
protected slots:
    void onRestBackgroundManager();
private:
    void init();
    BackgroundWidgetPointer createBackgroundWidget(ScreenPointer);
protected:
    DGioSettings *gsettings = nullptr;
    WMInter *wmInter = nullptr;
    DWindowManagerHelper* windowManagerHelper = nullptr;
private:
    bool m_preview = false;
    bool m_visible = true;
    int currentWorkspaceIndex = 0;

    QMap<ScreenPointer,BackgroundWidgetPointer> m_backgroundMap;
};

#endif // BACKGROUNDMANAGER_H
