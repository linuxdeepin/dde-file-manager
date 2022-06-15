/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef COLLECTIONTITLEBAR_P_H
#define COLLECTIONTITLEBAR_P_H

#include "collectiontitlebar.h"

#include <DStackedWidget>
#include <DLineEdit>
#include <DLabel>
#include <QPushButton>

#include <QHBoxLayout>

DDP_ORGANIZER_BEGIN_NAMESPACE

class CollectionTitleBarPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionTitleBarPrivate(CollectionTitleBar *qq = nullptr);
    ~CollectionTitleBarPrivate();

    void modifyTitleName();
    void titleNameModified();
    void updateDisplayName();
public:
    CollectionTitleBar *q = nullptr;
    QHBoxLayout *mainLayout = nullptr;
    Dtk::Widget::DLabel *nameLabel = nullptr;
    Dtk::Widget::DLineEdit *nameLineEdit = nullptr;
    Dtk::Widget::DStackedWidget *nameWidget = nullptr;
    Dtk::Widget::DPushButton *menuBtn = nullptr;

    bool canRename = true;
    QString titleName;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONTITLEBAR_P_H
