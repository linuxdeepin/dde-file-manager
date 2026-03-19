// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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

BasicStatusBarPrivate::~BasicStatusBarPrivate()
{
    discardCurrentJob();
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
    layout->setContentsMargins(0, 0, 4, 1);
}

void BasicStatusBarPrivate::calcFolderContains(const QList<QUrl> &folderList)
{
    discardCurrentJob();

    // If too many retired jobs are still stuck (typically due to network I/O),
    // stop scheduling new contains-scans to avoid unbounded resource growth.
    if (retiredJobs.size() >= kMaxRetiredJobs) {
        qCWarning(logDFMBase) << "BasicStatusBar: skip folder contains scan, retired jobs reached limit"
                              << retiredJobs.size() << ", limit:" << kMaxRetiredJobs
                              << ", request urls:" << folderList.size();
        showContains = false;
        q->updateStatusMessage();
        return;
    }

    auto *job = new FileScanner(this);
    job->setOptions(FileScanner::ScanOption::SingleDepth | FileScanner::ScanOption::CountOnly);
    finishedJobs.remove(job);

    connect(job, &FileScanner::finished, this, [this, job](const FileScanner::ScanResult &) {
        // `finished` can be emitted before worker thread fully stops. Track it
        // so retireJob() can release completed jobs without waiting for re-finished.
        finishedJobs.insert(job);
        if (retiredJobs.remove(job) > 0) {
            job->deleteLater();
        }
    });

    connect(job, &QObject::destroyed, this, [this, job]() {
        finishedJobs.remove(job);
        retiredJobs.remove(job);
    });

    fileStatisticsJog = job;
    ++scanGeneration;
    initJobConnection(job, scanGeneration);

    fileStatisticsJog->start(folderList);
}

void BasicStatusBarPrivate::initJobConnection(FileScanner *job, quint64 generation)
{
    if (!job)
        return;

    auto onFoundFile = [this, job, generation](const FileScanner::ScanResult &result) {
        if (generation != scanGeneration)
            return;

        if (job != fileStatisticsJog)
            return;

        int newCount = result.fileCount + result.directoryCount;
        if (newCount != folderContains) {
            folderContains = newCount;
            q->updateStatusMessage();
        }
    };

    connect(job, &FileScanner::progressChanged, this, onFoundFile);
}

void BasicStatusBarPrivate::retireJob(FileScanner *job)
{
    if (!job)
        return;

    // Detach from BasicStatusBar lifetime so blocked scanners won't be
    // synchronously destroyed on UI-thread during parent teardown.
    job->setParent(nullptr);

    if (job->isRunning() && !finishedJobs.contains(job)) {
        retiredJobs.insert(job);
        job->stop();
        return;
    }

    job->deleteLater();
}

void BasicStatusBarPrivate::discardCurrentJob()
{
    if (!fileStatisticsJog)
        return;

    auto *oldJob = fileStatisticsJog.data();
    fileStatisticsJog = nullptr;
    retireJob(oldJob);
}
