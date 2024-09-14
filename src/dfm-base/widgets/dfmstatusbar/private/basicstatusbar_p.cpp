// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basicstatusbar_p.h"
#include "widgets/dfmstatusbar/basicstatusbar.h"

#include <DHorizontalLine>
#include <DAnchors>

#include <QLabel>
#include <QHBoxLayout>

using namespace dfmbase;

BasicStatusBarPrivate::BasicStatusBarPrivate(BasicStatusBar *qq)
    : QObject(qq),
      q(qq)
{
    initFormatStrings();
}

void BasicStatusBarPrivate::initFormatStrings()
{
    onlyOneItemCounted = tr("%1 item");
    counted = tr("%1 items");
    onlyOneItemSelected = tr("%1 item selected");
    selected = tr("%1 items selected");
    selectOnlyOneFolder = tr("%1 folder selected (contains %2)");
    selectFolders = tr("%1 folders selected (contains %2)");
    selectOnlyOneFile = tr("%1 file selected (%2)");
    selectFiles = tr("%1 files selected (%2)");
    selectedNetworkOnlyOneFolder = tr("%1 folder selected");
}

void BasicStatusBarPrivate::initTipLabel()
{
    tip = new DTK_WIDGET_NAMESPACE::DTipLabel(counted.arg("0"), q);
    tip->setMinimumWidth(30);
    tip->setContentsMargins(0, 0, 0, 0);
    tip->setAlignment(Qt::AlignCenter);
    tip->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    tip->show();
}

void BasicStatusBarPrivate::initLayout()
{
    q->setFixedHeight(30);
    q->setContentsMargins(0, 0, 0, 0);
    auto vLayout = new QVBoxLayout(q);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    auto line = new DTK_WIDGET_NAMESPACE::DHorizontalLine(q);
    line->setContentsMargins(0, 0, 0, 0);
    line->setLineWidth(1);
    vLayout->addWidget(line);

    layout = new QHBoxLayout;
    vLayout->addLayout(layout);

    q->clearLayoutAndAnchors();
    layout->addWidget(tip);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 4, 0);
}

void BasicStatusBarPrivate::calcFolderContains(const QList<QUrl> &folderList)
{
    discardCurrentJob();

    fileStatisticsJog.reset(new FileStatisticsJob());
    fileStatisticsJog->setFileHints(FileStatisticsJob::kExcludeSourceFile | FileStatisticsJob::kSingleDepth);

    if (isJobDisconnect) {
        isJobDisconnect = false;
        initJobConnection();
    }

    fileStatisticsJog->start(folderList);
}

void BasicStatusBarPrivate::initJobConnection()
{
    if (!fileStatisticsJog)
        return;

    auto onFoundFile = [this](qint64 size, int filesCount, int directoryCount) {
        Q_UNUSED(size)

        if (!sender())
            return;

        if (filesCount + directoryCount != folderContains) {
            folderContains = filesCount + directoryCount;
            q->updateStatusMessage();
        }
    };

    auto currentJob = fileStatisticsJog;
    connect(currentJob.data(), &FileStatisticsJob::finished, this, [currentJob, this]() {
        folderContains = currentJob->filesCount() + currentJob->directorysCount();
        q->updateStatusMessage();
    });
    connect(currentJob.data(), &FileStatisticsJob::dataNotify, this, onFoundFile);
}

void BasicStatusBarPrivate::discardCurrentJob()
{
    if (!fileStatisticsJog)
        return;

    fileStatisticsJog->disconnect();
    isJobDisconnect = true;

    if (fileStatisticsJog->isRunning()) {
        auto waitDeletePointer = fileStatisticsJog;
        connect(waitDeletePointer.data(), &FileStatisticsJob::finished, this, [this, waitDeletePointer] {
            waitDeleteJobList.removeOne(waitDeletePointer);
        });
        fileStatisticsJog->stop();
        waitDeleteJobList.append(fileStatisticsJog);
    }

    fileStatisticsJog = nullptr;
}
