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

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QVBoxLayout;
QT_END_NAMESPACE

class Button;
class AppearanceDaemonInterface;
class WrapperWidget;
class WallpaperItem : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString data READ data WRITE setData)
    Q_PROPERTY(bool useThumbnailManager READ useThumbnailManager WRITE setUseThumbnailManager)

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

    QRect contentImageGeometry() const;

    void initPixmap();

    QString data() const;
    bool useThumbnailManager() const;

    QPushButton *addButton(const QString &id, const QString &text);

signals:
    void pressed();
    void hoverIn();
    void hoverOut();

    void buttonClicked(const QString &id);

public slots:
    void onThumbnailFounded(const QString &key, const QPixmap &pixmap);
    void thumbnailFinished();

    void setData(const QString &data);
    void setUseThumbnailManager(bool useThumbnailManager);

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    void refindPixmap();
    void onFindAborted(const QQueue<QString> &list);

    QString m_path;
    bool m_deletable;
    QString m_data;
    bool m_useThumbnailManager = true;

    QVBoxLayout *m_buttonLayout = nullptr;

    WrapperWidget * m_wrapper = NULL;
    QPropertyAnimation * m_upAnim = NULL;
    QPropertyAnimation * m_downAnim = NULL;

    QFutureWatcher<QPixmap> * m_thumbnailerWatcher = NULL;

    void initUI();
    void initAnimation();
};

#endif // WALLPAPERITEM_H
