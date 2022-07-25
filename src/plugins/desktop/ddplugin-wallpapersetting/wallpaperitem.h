/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
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
    inline QRect boxGeometry() const {
        return pixmapBoxGeometry;
    }
    void setBoxGeometry(const QRect &rect) {
        pixmapBoxGeometry = rect;
    }

    inline qreal opacity() const {
        return opacityValue;
    }
    inline void setOpacity(qreal opacity) {
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
    void enterEvent(QEvent *event) override;
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
    QMap<QPushButton*, QString> buttons;

    QGridLayout *buttonLayout = nullptr;

    WrapperWidget *wrapper = nullptr;
    QPropertyAnimation *upAnim = nullptr;
    QPropertyAnimation *downAnim = nullptr;
};

}

#endif // WALLPAPERITEM_H
