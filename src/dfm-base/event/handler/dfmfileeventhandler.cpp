/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "event/dfmglobaleventmessage.h"
#include "dfmfileeventhandler.h"

#include <QProcess>

#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include <cstdio>

DFMFileEventHandler::DFMFileEventHandler()
{

}

DFMFileEventHandler::~DFMFileEventHandler()
{

}

bool DFMFileEventHandler::canAsynProcess() {
    return true;
}

QString DFMFileEventHandler::scheme()
{
    return "file:///";
}

void DFMFileEventHandler::event(const DFMEventPointer &event)
{
    if (event->type() <= DFMEvent::FileEvent
            || event->type() >= DFMEvent::WindowEvent) {
        return;
    }

    DFMFileEventPointer e = qSharedPointerDynamicCast<DFMFileEvent>(event);
    if (e.isNull()) return;

    switch (e->type()) {

    case DFMFileEvent::CreateDir:
        createDir(e);
        break;

    case DFMFileEvent::CreateFile:
        createFile(e);
        break;

    case DFMFileEvent::DeleteDir:
        deleteDir(e);
        break;

    case DFMFileEvent::DeleteFile:
        deleteFile(e);
        break;

    case DFMFileEvent::MoveDir:
        moveDir(e);
        break;

    case DFMFileEvent::MoveFile:
        moveFile(e);
        break;

    case DFMFileEvent::RenameDir:
        renameDir(e);
        break;

    case DFMFileEvent::RenameFile:
        renameFile(e);
        break;

    case DFMFileEvent::CreateSymlink:
        createSymlink(e);
        break;

    default:
        qWarning() << "DFMFileEvent to process event is unknown type";
        break;
    }
}

void DFMFileEventHandler::createDir(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas()) {
        QUrl url = data.target();
        Q_ASSERT(url.isLocalFile());
        if (!QDir::current().mkdir(url.toLocalFile())) {

            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QString("error"),
                                 QString::fromLocal8Bit(strerror(errno)));
        }
    }
}

void DFMFileEventHandler::createFile(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas()) {
        QUrl url = data.target();

        Q_ASSERT(url.isLocalFile());
        QFile file(url.toLocalFile());

        if (file.exists()) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QString("warning"),
                                 QObject::tr("file exists: %0").arg(file.fileName()));
            continue;
        }

        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.close();
            continue;
        }

        Q_EMIT DFMGlobalEventMessage::globalInstance()
                ->eventError(QObject::tr("warning"),file.errorString());
    }
}

void DFMFileEventHandler::deleteDir(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;

    for (auto data: event->getDatas())
    {
        QUrl url = data.source();

        Q_ASSERT(url.isLocalFile());

        if (::rmdir(url.toLocalFile().toLocal8Bit()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),QString::fromLocal8Bit(strerror(errno)));

        }
    }
}

void DFMFileEventHandler::deleteFile(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas())
    {
        QUrl url = data.source();

        Q_ASSERT(url.isLocalFile());

        if (::rmdir(url.toLocalFile().toLocal8Bit()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),QString::fromLocal8Bit(strerror(errno)));

        }
    }
}

void DFMFileEventHandler::moveDir(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas()) {
        QUrl url = data.source();
        QUrl newUrl = data.target();

        Q_ASSERT(url.isLocalFile());
        Q_ASSERT(newUrl.isLocalFile());

        const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
        const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

        if (::rename(source_file.constData(), target_file.constData()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),
                                 QString::fromLocal8Bit(strerror(errno)));
        }
    }
}

void DFMFileEventHandler::moveFile(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas()) {
        QUrl url = data.source();
        QUrl newUrl = data.target();

        Q_ASSERT(url.isLocalFile());
        Q_ASSERT(newUrl.isLocalFile());

        const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
        const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

        if (::rename(source_file.constData(), target_file.constData()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),
                                 QString::fromLocal8Bit(strerror(errno)));
        }
    }
}

void DFMFileEventHandler::pasteFile(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas()) {
        QUrl url = data.source();
        QUrl newUrl = data.target();

        Q_ASSERT(url.isLocalFile());
        Q_ASSERT(newUrl.isLocalFile());

        const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
        const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

        QProcess::startDetached("cp -rf",{source_file,target_file});
    }
}

void DFMFileEventHandler::pasteDir(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;

    for (auto data: event->getDatas()) {
        QUrl url = data.source();
        QUrl newUrl = data.target();

        Q_ASSERT(url.isLocalFile());
        Q_ASSERT(newUrl.isLocalFile());

        const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
        const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

        QProcess::startDetached("cp -rf",{source_file,target_file});
    }
}

void DFMFileEventHandler::renameDir(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;
    for (auto data: event->getDatas()) {
        QUrl url = data.source();
        QUrl newUrl = data.target();

        Q_ASSERT(url.isLocalFile());
        Q_ASSERT(newUrl.isLocalFile());

        const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
        const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

        if (::rename(source_file.constData(), target_file.constData()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),
                                 QString::fromLocal8Bit(strerror(errno)));
        }
    }
}

void DFMFileEventHandler::renameFile(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;

    for (auto data: event->getDatas()) {
        QUrl url = data.source();
        QUrl newUrl = data.target();

        Q_ASSERT(url.isLocalFile());
        Q_ASSERT(newUrl.isLocalFile());

        const QByteArray &source_file = url.toLocalFile().toLocal8Bit();
        const QByteArray &target_file = newUrl.toLocalFile().toLocal8Bit();

        if (::rename(source_file.constData(), target_file.constData()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),
                                 QString::fromLocal8Bit(strerror(errno)));
        }
    }
}

void DFMFileEventHandler::createSymlink(const DFMFileEventPointer &event)
{
    qInfo() << Q_FUNC_INFO;

    for (auto data: event->getDatas()) {

        QUrl url = data.source();
        QUrl linkUrl = data.target();

        Q_ASSERT(linkUrl.isLocalFile());
        Q_ASSERT(linkUrl.isLocalFile());

        if (::symlink(url.toLocalFile().toLocal8Bit().constData(),
                      linkUrl.toLocalFile().toLocal8Bit().constData()) != 0) {
            Q_EMIT DFMGlobalEventMessage::globalInstance()
                    ->eventError(QObject::tr("error"),
                                 QString::fromLocal8Bit(strerror(errno)));
        }
    }
}
