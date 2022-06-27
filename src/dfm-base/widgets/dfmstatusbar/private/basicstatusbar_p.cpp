/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "basicstatusbar_p.h"
#include "widgets/dfmstatusbar/basicstatusbar.h"

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
    tip = new QLabel(counted.arg("0"), q);
    tip->setAlignment(Qt::AlignCenter);
    tip->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tip->show();
}

void BasicStatusBarPrivate::initLayout()
{
    layout = new QHBoxLayout(q);
    q->setLayout(layout);

    q->clearLayoutAndAnchors();
    layout->addWidget(tip);
    layout->setSpacing(14);
    layout->setContentsMargins(0, 0, 4, 0);
}

void BasicStatusBarPrivate::calcFolderContains(const QList<QUrl> &folderList)
{
    if (!fileStatisticsJog) {
        fileStatisticsJog = new FileStatisticsJob(this);
        fileStatisticsJog->setFileHints(FileStatisticsJob::kExcludeSourceFile | FileStatisticsJob::kSingleDepth);
    } else if (fileStatisticsJog->isRunning()) {
        fileStatisticsJog->stop();
        fileStatisticsJog->wait();
    }

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

        folderContains = filesCount + directoryCount;
        q->updateStatusMessage();
    };

    connect(fileStatisticsJog, &FileStatisticsJob::finished, this, [this]() {
        folderContains = fileStatisticsJog->filesCount() + fileStatisticsJog->directorysCount();
        q->updateStatusMessage();
    });
    connect(fileStatisticsJog, &FileStatisticsJob::dataNotify, this, onFoundFile);
}
