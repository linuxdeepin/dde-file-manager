// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEOPERATORHELPER_H
#define FILEOPERATORHELPER_H

#include "dfmplugin_sidebar_global.h"
#include <dfm-base/dfm_global_defines.h>

#include <QObject>
#include <QUrl>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarView;
class FileOperatorHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FileOperatorHelper)
public:
    static FileOperatorHelper *instance();

    void pasteFiles(quint64 windowId, const QList<QUrl> &srcUrls, const QUrl &targetUrl, const Qt::DropAction &action);

private:
    explicit FileOperatorHelper(QObject *parent = nullptr);
};

#define FileOperatorHelperIns DPSIDEBAR_USE_NAMESPACE::FileOperatorHelper::instance()

DPSIDEBAR_END_NAMESPACE

#endif   // FILEOPERATORHELPER_H
