#ifndef WALLPAPERLIST_H
#define WALLPAPERLIST_H

#include <QListWidget>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include <dwidget_global.h>
#include <anchors.h>

#include <com_deepin_wm.h>

DWIDGET_BEGIN_NAMESPACE
class DImageButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class WallpaperItem;
class AppearanceDaemonInterface;
class QGSettings;
class WallpaperList : public QListWidget
{
    Q_OBJECT

public:
    explicit WallpaperList(QWidget * parent = 0);
    ~WallpaperList();

    WallpaperItem * addWallpaper(const QString &path);
    void removeWallpaper(const QString &path);

    void scrollList(int step, int duration = 100);

    void prevPage();
    void nextPage();

signals:
    void wallpaperSet(QString wallpaper);
    void needCloseButton(QString path, QPoint pos) const;

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private:
    AppearanceDaemonInterface * m_dbusAppearance;
    com::deepin::wm *m_wmInter;
    QString m_oldWallpaperPath;
    QString m_oldLockPath;

    //It was handpicked item, Used for wallpaper page
    WallpaperItem *prevItem = Q_NULLPTR;
    WallpaperItem *nextItem = Q_NULLPTR;

    Anchors<DImageButton> prevButton;
    Anchors<DImageButton> nextButton;

    QPropertyAnimation scrollAnimation;

    void setWallpaper(QString realPath);
    void setLockScreen(QString realPath);

    void updateBothEndsItem();
    void showDeleteButtonForItem(const WallpaperItem *item) const;

    void recordOldValues();

private slots:
    void wallpaperItemPressed();
    void wallpaperItemHoverIn();
    void wallpaperItemHoverOut();
    void handleSetDesktop();
    void handleSetLock();
};

#endif // WALLPAPERLIST_H
