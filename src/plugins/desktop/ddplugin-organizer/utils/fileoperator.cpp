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
#include "fileoperator_p.h"

#include "dfm-base/dfm_event_defines.h"

#include <QUrl>

using namespace ddplugin_organizer;
using namespace dfmbase;

class FileOperatorGlobal : public FileOperator {};
Q_GLOBAL_STATIC(FileOperatorGlobal, fileOperatorGlobal)

FileOperatorPrivate::FileOperatorPrivate(FileOperator *qq)
    : q(qq)
{

}

FileOperator::FileOperator(QObject *parent)
    : QObject(parent)
    , d(new FileOperatorPrivate(this))
{

}

FileOperator::~FileOperator()
{

}

FileOperator *FileOperator::instance()
{
    return fileOperatorGlobal;
}

void FileOperator::dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    // todo
}

void FileOperator::dropToTrash(const QList<QUrl> &urls)
{
    dpfSignalDispatcher->publish(GlobalEventType::kMoveToTrash, 0, urls, AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperator::dropToApp(const QList<QUrl> &urls, const QString &app)
{
    QList<QString> apps { app };
    dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, urls, apps);
}

