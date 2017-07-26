#ifndef WALLPAPERLIST_H
#define WALLPAPERLIST_H

#include <QScrollArea>
#include <QPropertyAnimation>

#include <dwidget_global.h>
#include <anchors.h>

#include <com_deepin_wm.h>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE
class DImageButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class WallpaperItem;
class AppearanceDaemonInterface;
class QGSettings;
class WallpaperList : public QScrollArea
{
    Q_OBJECT

public:
    explicit WallpaperList(QWidget * parent = 0);
    ~WallpaperList();

    WallpaperItem * addWallpaper(const QString &path);
    WallpaperItem * getWallpaperByPath(const QString &path) const;
    void removeWallpaper(const QString &path);

    void scrollList(int step, int duration = 100);

    void prevPage();
    void nextPage();

    QString desktopWallpaper() const;
    QString lockWallpaper() const;

    QSize gridSize() const;
    void setGridSize(const QSize &size);

    void addItem(QWidget *item);
    QWidget *item(int index) const;
    QWidget *itemAt(const QPoint &pos) const;
    QWidget *itemAt(int x, int y) const;
    void removeItem(QWidget *item);
    void removeItem(int index);
    int count() const;

    void clear();

signals:
    void needPreviewWallpaper(QString path) const;
    void needCloseButton(QString path, QPoint pos) const;

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    QWidget *m_contentWidget;
    QHBoxLayout *m_contentLayout;

    AppearanceDaemonInterface * m_dbusAppearance;
    com::deepin::wm *m_wmInter;

    QString m_desktopWallpaper;
    QString m_lockWallpaper;

    //It was handpicked item, Used for wallpaper page
    WallpaperItem *prevItem = Q_NULLPTR;
    WallpaperItem *nextItem = Q_NULLPTR;

    Anchors<DImageButton> prevButton;
    Anchors<DImageButton> nextButton;

    QPropertyAnimation scrollAnimation;

    QSize m_gridSize;

    void updateBothEndsItem();
    void showDeleteButtonForItem(const WallpaperItem *item) const;

private slots:
    void wallpaperItemPressed();
    void wallpaperItemHoverIn();
    void wallpaperItemHoverOut();
    void handleSetDesktop();
    void handleSetLock();
};

#endif // WALLPAPERLIST_H
