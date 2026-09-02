// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "trashcorehelper.h"
#include "views/trashpropertydialog.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/trashutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/dpf.h>

#include <QDir>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_trashcore;

QUrl TrashCoreHelper::rootUrl()
{
    return TrashUtils::trashRootUrl();
}

QWidget *TrashCoreHelper::createTrashPropertyDialog(const QUrl &url)
{
    static TrashPropertyDialog *trashPropertyDialog = nullptr;
    if (UniversalUtils::urlEquals(url, TrashUtils::trashRootUrl()) || FileUtils::isTrashDesktopFile(url)) {
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
    return std::make_pair(TrashUtils::calculateTrashSize(), TrashUtils::countTrashItems());
}
