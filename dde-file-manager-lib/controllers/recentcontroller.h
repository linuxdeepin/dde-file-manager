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

#ifndef RECENTCONTROLLER_H
#define RECENTCONTROLLER_H

#include "dabstractfilecontroller.h"

class DAbstractFileInfo;
class DFileWatcher;

class RecentController : public DAbstractFileController
{
    Q_OBJECT

public:
    explicit RecentController(QObject *parent = nullptr);

    bool openFile(const QSharedPointer<DFMOpenFileEvent> &event) const override;
    bool openFileLocation(const QSharedPointer<DFMOpenFileLocation> &event) const override;

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;
    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvnet> &event) const override;

    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

private:
    DFileWatcher* m_fileWatcher;
};

#endif // RECENTCONTROLLER_H
