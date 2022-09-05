// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesignalmanager.h"

#include "dfmevent.h"
#include "dabstractfileinfo.h"
#include "durl.h"
#include <QDir>


FileSignalManager::FileSignalManager(QObject *parent):
    QObject(parent)
{
    qRegisterMetaType<DFMEvent>(QT_STRINGIFY(DFMEvent));
    qRegisterMetaType<DFMUrlListBaseEvent>(QT_STRINGIFY(DFMUrlListBaseEvent));
    qRegisterMetaType<QDir::Filters>(QT_STRINGIFY(QDir::Filters));
    qRegisterMetaType<QList<DAbstractFileInfoPointer>>(QT_STRINGIFY(QList<DAbstractFileInfoPointer>));
    qRegisterMetaType<DUrl>(QT_STRINGIFY(DUrl));
    qRegisterMetaType<DUrl>();
    qRegisterMetaType<DUrlList>();
}
