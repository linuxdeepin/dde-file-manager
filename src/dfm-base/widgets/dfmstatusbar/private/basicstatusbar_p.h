// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICSTATUSBAR_P_H
#define BASICSTATUSBAR_P_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/filescanner.h>

#include <DAnchors>
#include <DTipLabel>

#include <QObject>
#include <QString>
#include <QPointer>
#include <QSet>

class QLabel;
class QHBoxLayout;

namespace dfmbase {

class BasicStatusBar;
class BasicStatusBarPrivate : public QObject
{
    Q_OBJECT
    friend class BasicStatusBar;
    BasicStatusBar *const q;

public:
    explicit BasicStatusBarPrivate(BasicStatusBar *qq);
    ~BasicStatusBarPrivate() override;

    void initFormatStrings();

    void initTipLabel();
    void initLayout();

    void calcFolderContains(const QList<QUrl> &folderList);
    void discardCurrentJob();
    void initJobConnection(FileScanner *job, quint64 generation);
    void retireJob(FileScanner *job);

    QString onlyOneItemCounted;
    QString counted;
    QString onlyOneItemSelected;
    QString selected;

    QString selectFolders;
    QString selectOnlyOneFolder;
    QString selectFiles;
    QString selectOnlyOneFile;
    QString selectedNetworkOnlyOneFolder;

    bool showContains { true };

    int fileCount = 0;
    qint64 fileSize = 0;
    int folderCount = 0;
    int folderContains = 0;

    QHBoxLayout *layout = nullptr;
    DTK_WIDGET_NAMESPACE::DTipLabel *tip = nullptr;


    QPointer<FileScanner> fileStatisticsJog;
    QSet<FileScanner *> retiredJobs;
    QSet<FileScanner *> finishedJobs;
    quint64 scanGeneration = 0;
    static constexpr int kMaxRetiredJobs = 2;
};

}

#endif   // BASICSTATUSBARPRIVATE_H
