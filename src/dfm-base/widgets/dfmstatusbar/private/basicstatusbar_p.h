// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICSTATUSBAR_P_H
#define BASICSTATUSBAR_P_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DAnchors>
#include <DTipLabel>

#include <QObject>
#include <QString>

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

    void initFormatStrings();

    void initTipLabel();
    void initLayout();

    void calcFolderContains(const QList<QUrl> &folderList);
    void initJobConnection();
    void discardCurrentJob();

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


    QSharedPointer<FileStatisticsJob> fileStatisticsJog;
    bool isJobDisconnect = true;
    QList<QSharedPointer<FileStatisticsJob>> waitDeleteJobList {};
};

}

#endif   // BASICSTATUSBARPRIVATE_H
