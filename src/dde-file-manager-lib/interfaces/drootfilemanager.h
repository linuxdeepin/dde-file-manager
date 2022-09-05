// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    bool isRootFileContainSmb(const DUrl &smburl);
signals:
    void rootFileChange(const DAbstractFileInfoPointer &chi) const;
    void queryRootFileFinsh() const;
    void serviceHideSystemPartition() const;

public Q_SLOTS:
    void hideSystemPartition();
    void policyHideSystemPartition(const QString &key);
    void settingHideSystemPartition(bool isHide);


private:
    explicit DRootFileManager(QObject *parent = nullptr);
    ~DRootFileManager();

    QScopedPointer<DRootFileManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DRootFileManager)
};

#endif // FROOTFILEMANAGER_H
