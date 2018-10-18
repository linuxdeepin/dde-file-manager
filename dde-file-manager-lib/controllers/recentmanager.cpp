/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "recentmanager.h"
#include "dfileservices.h"
#include "dfilewatcher.h"
#include "dfmevent.h"
#include "models/recentfileinfo.h"
#include "private/dabstractfilewatcher_p.h"

#include <qdebug.h>
#include <qxmlstream.h>

class RecentFileWatcherPrivate : public DAbstractFileWatcherPrivate
{
public:
    RecentFileWatcherPrivate(DAbstractFileWatcher *qq)
        : DAbstractFileWatcherPrivate(qq) {}

    bool start() override
    {
        started = true;

        return true;
    }

    bool stop() override
    {
        started = false;

        return true;
    }
};

class RecentFileWatcher : public DAbstractFileWatcher
{
public:
    explicit RecentFileWatcher(const DUrl &url, QObject *parent = nullptr)
        : DAbstractFileWatcher(*new RecentFileWatcherPrivate(this), url, parent)
    {

    }
};

RecentController::RecentController(QObject *parent)
    : DAbstractFileController(parent),
      m_fileWatcher(new DFileWatcher(QDir::homePath() + "/.local/share/recently-used.xbel", this))
{
    m_fileWatcher->startWatcher();

    connect(m_fileWatcher, &DFileWatcher::fileModified, this, [=] {

    });
}

bool RecentController::openFile(const QSharedPointer<DFMOpenFileEvent> &event) const
{
    return DFileService::instance()->openFile(event->sender(), DUrl(event->url().fragment()));
}

bool RecentController::openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const
{
    return DFileService::instance()->openFileLocation(event->sender(), DUrl(event->url().fragment()));
}

const QList<DAbstractFileInfoPointer> RecentController::getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const
{
    QList<DAbstractFileInfoPointer> list;

    QFile file(QDir::homePath() + "/.local/share/recently-used.xbel");
    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);

        while (!reader.atEnd()) {
            if (!reader.readNextStartElement() ||
                 reader.name() != "bookmark") {
                continue;
            }

            const auto &location = reader.attributes().value("href");
            if (!location.isEmpty()) {
                DUrl url = DUrl(location.toString());
                QFileInfo info(url.toLocalFile());

                if (info.exists() && info.isFile()) {
                    list << DAbstractFileInfoPointer(new RecentFileInfo(url));
                }
            }
        }
    }

    return list;
}

const DAbstractFileInfoPointer RecentController::createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const
{
    return DAbstractFileInfoPointer(new RecentFileInfo(event->url()));
}

DAbstractFileWatcher *RecentController::createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const
{
    return new DFileWatcher(QDir::homePath() + "/.local/share/recently-used.xbel");
}
