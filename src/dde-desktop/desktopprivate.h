#ifndef DESKTOPPRIVATE_H
#define DESKTOPPRIVATE_H

#include "../dde-wallpaper-chooser/frame.h"


#ifndef DISABLE_ZONE
#include "../dde-zone/mainwindow.h"
#endif
#include <malloc.h>

using WallpaperSettings = Frame;

#ifndef DISABLE_ZONE
using ZoneSettings = ZoneMainWindow;
#endif

class BackgroundManager;
class CanvasViewManager;
class DesktopPrivate
{
public:
    ~DesktopPrivate()
    {
        if (m_background) {
            delete m_background;
            m_background = nullptr;
        }

        if (m_canvas) {
            delete m_canvas;
            m_canvas = nullptr;
        }

        if (wallpaperSettings) {
            delete wallpaperSettings;
            wallpaperSettings = nullptr;
        }
#ifndef DISABLE_ZONE
        if (zoneSettings) {
            delete zoneSettings;
            zoneSettings = nullptr;
        }
#endif
    }
    BackgroundManager *m_background = nullptr;
    CanvasViewManager *m_canvas = nullptr;
    WallpaperSettings *wallpaperSettings = nullptr;

#ifndef DISABLE_ZONE
    ZoneSettings *zoneSettings { nullptr };
#endif
};
#endif // DESKTOPPRIVATE_H
