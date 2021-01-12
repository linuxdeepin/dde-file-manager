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
    WallpaperItem(QFrame *parent = nullptr, const QString &path = "");
    ~WallpaperItem() override;

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
    void tab();
    void backtab();

    void buttonClicked(const QString &id);

public slots:
    void onThumbnailFounded(const QString &key, const QPixmap &pixmap);
    void thumbnailFinished();

    void setData(const QString &data);
    void setUseThumbnailManager(bool useThumbnailManager);

    void focusLastButton();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void refindPixmap();
    void onFindAborted(const QQueue<QString> &list);

    QString m_path;
    bool m_deletable;
    QString m_data;
    bool m_useThumbnailManager = true;

    QVBoxLayout *m_buttonLayout = nullptr;

    WrapperWidget *m_wrapper = nullptr;
    QPropertyAnimation *m_upAnim = nullptr;
    QPropertyAnimation *m_downAnim = nullptr;

    QFutureWatcher<QPixmap> *m_thumbnailerWatcher = nullptr;

    void initUI();
    void initAnimation();
};

#endif // WALLPAPERITEM_H
