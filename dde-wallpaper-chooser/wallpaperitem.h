/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include <QFrame>
#include <QFutureWatcher>
#include <QPropertyAnimation>

class QLabel;
class Button;
class AppearanceDaemonInterface;
class WrapperWidget;
class WallpaperItem : public QFrame
{
    Q_OBJECT
public:
    WallpaperItem(QFrame *parent = 0, const QString &path = "");
    ~WallpaperItem();

    void slideUp();
    void slideDown();

    QString getPath() const;
    void setPath(const QString &path);

    bool getDeletable() const;
    void setDeletable(bool deletable);

    void setOpacity(qreal opacity);

    QRect conentImageGeometry() const;

    void initPixmap();

signals:
    void pressed();
    void hoverIn();
    void hoverOut();
    void desktopButtonClicked();
    void lockButtonClicked();

public slots:
    void onThumbnailFounded(const QString &key, const QPixmap &pixmap);
    void thumbnailFinished();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    QString m_path;
    bool m_deletable;

    WrapperWidget * m_wrapper = NULL;
    Button * m_desktopButton = NULL;
    Button * m_lockButton = NULL;
    QPropertyAnimation * m_upAnim = NULL;
    QPropertyAnimation * m_downAnim = NULL;

    QFutureWatcher<QPixmap> * m_thumbnailerWatcher = NULL;

    void initUI();
    void initAnimation();
};

#endif // WALLPAPERITEM_H
