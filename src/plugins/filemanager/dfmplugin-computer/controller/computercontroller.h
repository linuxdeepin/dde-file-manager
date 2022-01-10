/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef COMPUTERCONTROLLER_H
#define COMPUTERCONTROLLER_H

#include "dfmplugin_computer_global.h"

#include "dfm-base/file/entry/entryfileinfo.h"

#include <QUrl>
#include <QObject>

#define ComputerControllerInstance DPCOMPUTER_NAMESPACE::ComputerController::instance()

DPCOMPUTER_BEGIN_NAMESPACE

class ComputerController : public QObject
{
    Q_OBJECT
public:
    static ComputerController *instance();

    static void cdTo(quint64 winId, const QUrl &url);
    static void requestMenu(quint64 winId, const QUrl &url, const QPoint &pos);
    static void rename(quint64 winId, const QUrl &url, const QString &name);

    static void mountAndEnterBlockDevice(quint64 winId, const DFMEntryFileInfoPointer info);
    static void mountAndEnterBlockDevice(quint64 winId, const QString &id);

private:
    explicit ComputerController(QObject *parent = nullptr);
};

DPCOMPUTER_END_NAMESPACE

#endif   // COMPUTERCONTROLLER_H
