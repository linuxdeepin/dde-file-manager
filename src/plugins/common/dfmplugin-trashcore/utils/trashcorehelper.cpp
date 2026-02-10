// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcorehelper.h"
#include "views/trashpropertydialog.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include <dfm-io/denumerator.h>

#include <QDir>
#include <QDirIterator>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trashcore;

QUrl TrashCoreHelper::rootUrl()
{
    QUrl url;
    url.setScheme(scheme());
    url.setPath("/");
    return url;
}

QWidget *TrashCoreHelper::createTrashPropertyDialog(const QUrl &url)
{
    static TrashPropertyDialog *trashPropertyDialog = nullptr;
    if (UniversalUtils::urlEquals(url, FileUtils::trashRootUrl()) || FileUtils::isTrashDesktopFile(url)) {
        if (!trashPropertyDialog) {
            trashPropertyDialog = new TrashPropertyDialog();
            return trashPropertyDialog;
        }
        return trashPropertyDialog;
    }
    return nullptr;
}

std::pair<qint64, int> TrashCoreHelper::calculateTrashRoot()
{
    qint64 size = 0;
    int count = 0;
    QList<QUrl> files;
    DFMIO::DEnumerator enumerator(FileUtils::trashRootUrl());
    while (enumerator.hasNext()) {
        const QUrl &urlNext = enumerator.next();
        if (files.contains(FileUtils::bindUrlTransform(urlNext)))
            continue;
        files << FileUtils::bindUrlTransform(urlNext);
        ++count;
        FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(urlNext);
        if (!fileInfo)
            continue;
        size += fileInfo->size();
    }
    return std::make_pair<qint64, int>(qint64(size), int(count));
}
