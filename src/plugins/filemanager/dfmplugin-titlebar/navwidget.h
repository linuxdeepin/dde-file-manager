/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef NavWidget_H
#define NavWidget_H

#include <DButtonBox>

#include <QUrl>
#include <QWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class NavWidgetPrivate;
class NavWidget : public QWidget
{
    Q_OBJECT
    friend class NavWidgetPrivate;
    NavWidgetPrivate *const d;

public:
    explicit NavWidget(QWidget *parent = nullptr);
    DButtonBoxButton *navBackButton() const;
    DButtonBoxButton *navForwardButton() const;
    void setNavBackButton(DButtonBoxButton *navBackButton);
    void setNavForwardButton(DButtonBoxButton *navForwardButton);
public Q_SLOTS:
    void appendUrl(const QUrl &url);
Q_SIGNALS:
    void releaseUrl(const QUrl &url);
};

#endif   // NavWidget_H
