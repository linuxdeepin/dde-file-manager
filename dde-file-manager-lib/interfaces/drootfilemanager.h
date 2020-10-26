/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *               2020        max-lv
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *             max-lv<lvwujun@uniontech.com>
 *
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

#ifndef FROOTFILEMANAGER_H
#define FROOTFILEMANAGER_H

#include "dabstractfilecontroller.h"
#include "durl.h"

#include <QObject>
#include <QString>
#include <QMultiHash>
#include <QPair>
#include <QDir>

#include <functional>

DFM_USE_NAMESPACE

class JobController;
class DRootFileManagerPrivate;

class DRootFileManager : public QObject
{
    Q_OBJECT

public:
    static DRootFileManager *instance();

    QList<DAbstractFileInfoPointer> getRootFile();
    bool isRootFileInited() const;
    void changeRootFile(const DUrl &fileurl,const bool bcreate = true);
    void startQuryRootFile();
    DAbstractFileWatcher *rootFileWather() const;
    void clearThread();
    //chang rootfile
    void changRootFile(const QList<DAbstractFileInfoPointer> &rootinfo);
    //处理rootfilelist中是否包含某个durl
    bool isRootFileContain(const DUrl &url);
    //get root file info cache
    static const DAbstractFileInfoPointer getFileInfo(const DUrl &fileUrl);

signals:
    void rootFileChange(const DAbstractFileInfoPointer &chi) const;
    void queryRootFileFinsh() const;
    void serviceHideSystemPartition() const;

public Q_SLOTS:
    void hideSystemPartition();

private:
    explicit DRootFileManager(QObject *parent = nullptr);
    ~DRootFileManager();

    QScopedPointer<DRootFileManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DRootFileManager)
};

#endif // FROOTFILEMANAGER_H
