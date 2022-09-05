// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMROOTCONTROLLER_H
#define DFMROOTCONTROLLER_H

#include "dabstractfilecontroller.h"
#include "dabstractfilewatcher.h"
#include "durl.h"
#include <dgiomount.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>

#define DISK_HIDDEN "dfm.disk.hidden"
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

    static QStringList systemDiskList();

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
