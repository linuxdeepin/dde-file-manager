/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QSharedPointer>

class CanvasGridView;
class BackgroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BackgroundWidget(QWidget *parent = nullptr);
    ~BackgroundWidget() override;
    void setPixmap(const QPixmap &pixmap);
    void paintEvent(QPaintEvent *event) override;
    void setView(const QSharedPointer<CanvasGridView> &);
    inline QPixmap pixmap() const {return m_pixmap;}
    void setAccessableInfo(const QString& info);
private:
    QPixmap m_pixmap;
    QPixmap m_noScalePixmap;
    QSharedPointer<CanvasGridView> m_view; //保有view的指针，防止背景释放时，连带一起释放view
};

typedef QSharedPointer<BackgroundWidget> BackgroundWidgetPointer;
#endif // BACKGROUNDWIDGET_H
