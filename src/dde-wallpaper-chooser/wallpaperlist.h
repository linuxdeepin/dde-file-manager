// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERLIST_H
#define WALLPAPERLIST_H

#include <QScrollArea>
#include <QPropertyAnimation>

#include <dtkwidget_global.h>
#include <danchors.h>

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
class WallpaperList : public QScrollArea
{
    Q_OBJECT

public:
    explicit WallpaperList(QWidget *parent = 0);
    ~WallpaperList() override;

    WallpaperItem *addWallpaper(const QString &path);
    void removeWallpaper(const QString &path);

    void scrollList(int step, int duration = 100);

    void prevPage();
    void nextPage();

    QSize gridSize() const;
    void setGridSize(const QSize &size);

    void addItem(WallpaperItem *item);
    QWidget *item(int index) const;
    QWidget *itemAt(const QPoint &pos) const;
    QWidget *itemAt(int x, int y) const;
    int count() const;

    void clear();

    void updateItemThumb();
    void setCurrentIndex(int index);
    WallpaperItem *getCurrentItem();

signals:
    void itemPressed(QString data) const;
    void mouseOverItemChanged(QString path, QPoint pos) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QWidget *m_contentWidget;
    QHBoxLayout *m_contentLayout;

    com::deepin::wm *m_wmInter;

    //It was handpicked item, Used for wallpaper page
    WallpaperItem *prevItem = Q_NULLPTR;
    WallpaperItem *nextItem = Q_NULLPTR;

    DAnchors<DImageButton> prevButton;
    DAnchors<DImageButton> nextButton;

    QPropertyAnimation scrollAnimation;

    QSize m_gridSize;

    QList<WallpaperItem *> m_items;

    QTimer *m_updateTimer;

    void updateBothEndsItem();
    void showDeleteButtonForItem(const WallpaperItem *item) const;

    friend class Frame;
    int m_index = 0;

private slots:
    void wallpaperItemPressed();
    void wallpaperItemHoverIn();
    void wallpaperItemHoverOut();
};

#endif // WALLPAPERLIST_H
