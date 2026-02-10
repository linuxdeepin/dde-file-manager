// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filefilter.h"
#include "fileprovider.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/standardpaths.h>

#include <QDateTime>
#include <QTimerEvent>
#include <QDebug>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

FileFilter::FileFilter()
{
}

FileFilter::~FileFilter()
{
}

bool FileFilter::fileTraversalFilter(QList<QUrl> &urls)
{
    return false;
}

bool FileFilter::fileDeletedFilter(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

bool FileFilter::fileCreatedFilter(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

bool FileFilter::fileRenameFilter(const QUrl &oldUrl, const QUrl &newUrl)
{
    Q_UNUSED(oldUrl)
    Q_UNUSED(newUrl)
    return false;
}

bool FileFilter::fileUpdatedFilter(const QUrl &url)
{
    Q_UNUSED(url)
    return false;
}

#define UpdateInterval 500

RedundantUpdateFilter::RedundantUpdateFilter(FileProvider *parent)
    : QObject(parent), FileFilter(), provider(parent)
{
}

bool RedundantUpdateFilter::fileUpdatedFilter(const QUrl &url)
{
    auto itor = updateList.find(url);
    if (itor == updateList.end()) {
        updateList.insert(url, 0);
        return false;
    }

    itor.value()++;
    if (timerid < 0)
        timerid = startTimer(UpdateInterval);

    return true;
}

void RedundantUpdateFilter::checkUpdate()
{
    QList<QUrl> toUpdate;
    for (auto itor = updateList.begin(); itor != updateList.end(); ++itor) {
        if (itor.value() != 0)
            toUpdate.append(itor.key());
    }

    updateList.clear();
    killTimer(timerid);
    timerid = -1;

    for (const QUrl &url : toUpdate)
        emit provider->fileUpdated(url);
}

void RedundantUpdateFilter::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerid) {
        checkUpdate();
        event->accept();
        return;
    }

    return QObject::timerEvent(event);
}
