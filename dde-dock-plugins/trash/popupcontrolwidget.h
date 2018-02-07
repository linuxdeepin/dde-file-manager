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

#ifndef POPUPCONTROLWIDGET_H
#define POPUPCONTROLWIDGET_H

#include <QWidget>
#include <QFileSystemWatcher>

#include <dlinkbutton.h>

class PopupControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PopupControlWidget(QWidget *parent = 0);

    bool empty() const;
    int trashItems() const;
    QSize sizeHint() const;
    static const QString trashDir();

public slots:
    void openTrashFloder();
    void clearTrashFloder();

signals:
    void emptyChanged(const bool empty) const;

private:
    int trashItemCount() const;

private slots:
    void trashStatusChanged();

private:
    bool m_empty;
    int m_trashItemsCount;

//    Dtk::Widget::DLinkButton *m_openBtn;
//    Dtk::Widget::DLinkButton *m_clearBtn;

    QFileSystemWatcher *m_fsWatcher;
};

#endif // POPUPCONTROLWIDGET_H
