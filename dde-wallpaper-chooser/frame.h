/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FRAME_H
#define FRAME_H

#include <dblureffectwidget.h>
#include <dimagebutton.h>
#include <dregionmonitor.h>

DWIDGET_USE_NAMESPACE

class WallpaperList;
class WallpaperListView;
class AppearanceDaemonInterface;
class DeepinWM;
class Frame : public DBlurEffectWidget
{
    Q_OBJECT

public:
    Frame(QFrame *parent = 0);
    ~Frame();

    void show();

signals:
    void done();

public slots:
    void handleNeedCloseButton(QString path, QPoint pos);

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void keyPressEvent(QKeyEvent *);

private:
    WallpaperList *m_wallpaperList = NULL;
    DImageButton * m_closeButton = NULL;

    AppearanceDaemonInterface * m_dbusAppearance = NULL;
    DeepinWM * m_dbusDeepinWM = NULL;
    DRegionMonitor * m_mouseArea = NULL;

    QString m_formerWallpaper;
    QMap<QString, bool> m_deletableInfo;

    void initSize();
    void initListView();
    void refreshList();
    QStringList processListReply(const QString &reply);
};

#endif // FRAME_H
