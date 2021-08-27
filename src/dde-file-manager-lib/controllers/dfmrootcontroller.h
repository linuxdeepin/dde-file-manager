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

#ifndef DFMROOTCONTROLLER_H
#define DFMROOTCONTROLLER_H

#include "dabstractfilecontroller.h"
#include "dabstractfilewatcher.h"
#include "durl.h"
#include <dgiomount.h>
#include <dblockdevice.h>

class DFMRootFileInfo;
class DFMRootFileWatcherPrivate;
class DFMRootFileWatcher : public DAbstractFileWatcher
{
    Q_OBJECT

public:
    explicit DFMRootFileWatcher(const DUrl &url, QObject *parent = nullptr);

private:
    Q_DECLARE_PRIVATE(DFMRootFileWatcher)
};

class DFMRootController : public DAbstractFileController
{
    Q_OBJECT
public:
    explicit DFMRootController(QObject *parent = nullptr);
    bool renameFile(const QSharedPointer<DFMRenameEvent> &event) const override;

    const QList<DAbstractFileInfoPointer> getChildren(const QSharedPointer<DFMGetChildrensEvent> &event) const override;

    const DAbstractFileInfoPointer createFileInfo(const QSharedPointer<DFMCreateFileInfoEvent> &event) const override;
    DAbstractFileWatcher *createFileWatcher(const QSharedPointer<DFMCreateFileWatcherEvent> &event) const override;

private:
    void reloadBlkName(const QString& blkPath, QSharedPointer<DBlockDevice> blk) const;

    /**
     * @brief loadDiskInfo 加载磁盘信息
     * @param jsonPath 磁盘信息文件路径
     */
    void loadDiskInfo(const QString &jsonPath) const;


    /**
     * @brief 为分区盘符设置一种别名，用于在计算机页面显示
     * @param fi
     * @param alias
     * @return
     */
    bool setLocalDiskAlias(DFMRootFileInfo *fi, const QString &alias) const;

};

#endif // DFMROOTCONTROLLER_H
