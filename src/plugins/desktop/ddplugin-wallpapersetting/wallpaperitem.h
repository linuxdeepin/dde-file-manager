// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WALLPAPERITEM_H
#define WALLPAPERITEM_H

#include "ddplugin_wallpapersetting_global.h"

#include <QString>
#include <QFrame>
#include <QFutureWatcher>
#include <QPropertyAnimation>

class QPushButton;
class QGridLayout;

namespace ddplugin_wallpapersetting {

class WrapperWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WrapperWidget(QWidget *parent = nullptr);
    void setPixmap(const QPixmap &pix);
    inline QRect boxGeometry() const
    {
        return pixmapBoxGeometry;
    }
    void setBoxGeometry(const QRect &rect)
    {
        pixmapBoxGeometry = rect;
    }

    inline qreal opacity() const
    {
        return opacityValue;
    }
    inline void setOpacity(qreal opacity)
    {
        opacityValue = opacity;
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap pixmap;
    qreal opacityValue = 1;
    QRect pixmapBoxGeometry;
};
class WallpaperItem : public QFrame
{
    Q_OBJECT
public:
    explicit WallpaperItem(QWidget *parent = nullptr);
    ~WallpaperItem();

    QString itemData() const;
    void setItemData(const QString &data);
    QString sketch() const;
    void setSketch(const QString &url);
    bool enableThumbnail() const;
    void setEnableThumbnail(bool);
    bool isDeletable() const;
    void setDeletable(bool del);
    void setOpacity(qreal opacity);
    void slideUp();
    void slideDown();
    void renderPixmap();
    QRect contentGeometry() const;
    QPushButton *addButton(const QString &id, const QString &text, const int btnWidth, int row, int column, int rowSpan, int columnSpan);
    void setEntranceIconOfSettings(const QString &id);
signals:
    void pressed(WallpaperItem *self);
    void hoverIn(WallpaperItem *self);
    void hoverOut(WallpaperItem *self);
    void tab(WallpaperItem *self);
    void backtab(WallpaperItem *self);

    void buttonClicked(WallpaperItem *self, const QString &id);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *event) override;
#else
    void enterEvent(QEnterEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void refindPixmap();
    void focusOnLastButton();
    void focusOnFirstButton();
    QString thumbnailKey() const;

private:
    void init();
private slots:
    void onButtonClicked();
    void onThumbnailFounded(const QString &key, QPixmap pixmap);
    void onFindAborted(const QQueue<QString> &list);

private:
    QString sketchPath;
    QString itData;
    bool enablethumbnail = true;
    bool deletable = false;
    QFutureWatcher<QPixmap> *thumbnailerWatcher = nullptr;
    QMap<QPushButton *, QString> buttons;

    QGridLayout *buttonLayout = nullptr;

    WrapperWidget *wrapper = nullptr;
    QPropertyAnimation *upAnim = nullptr;
    QPropertyAnimation *downAnim = nullptr;
};

}

#endif   // WALLPAPERITEM_H
