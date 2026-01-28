// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filecleanupmanager.h"

#include <dfm-base/file/local/localfilehandler.h>

DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

void FileCleanupManager::trackIncompleteFile(const QUrl &url)
{
    if (!url.isValid())
        return;

    if (!incompleteFiles.contains(url)) {
        incompleteFiles.append(url);
        fmDebug() << "Track incomplete file:" << url.toLocalFile();
    }
}

void FileCleanupManager::confirmCompleted(const QUrl &url)
{
    if (incompleteFiles.removeAll(url) > 0) {
        fmDebug() << "Confirm file completed:" << url.toLocalFile();
    }
}

void FileCleanupManager::cleanupIncompleteFiles()
{
    if (incompleteFiles.isEmpty())
        return;

    fmInfo() << "Cleaning up" << incompleteFiles.count() << "incomplete files";

    for (const QUrl &url : incompleteFiles) {
        fmInfo() << "Deleted incomplete file:" << url.toLocalFile();
        if (!LocalFileHandler().deleteFile(url)) {
            fmWarning() << "Failed to cleanup incomplete file" << url;
        }
    }

    incompleteFiles.clear();
}
