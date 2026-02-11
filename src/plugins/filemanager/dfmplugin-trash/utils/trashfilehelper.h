// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHFILEHELPER_H
#define TRASHFILEHELPER_H

#include "dfmplugin_trash_global.h"

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/utils/clipboard.h>

#include <QObject>
#include <QUrl>

DPTRASH_BEGIN_NAMESPACE

class TrashFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashFileHelper)
public:
    static TrashFileHelper *instance();
    inline static QString scheme()
    {
        return "trash";
    }

    bool cutFile(const quint64 windowId, const QList<QUrl> sources,
                 const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool copyFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                  const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool moveToTrash(const quint64 windowId, const QList<QUrl> sources,
                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool deleteFile(const quint64 windowId, const QList<QUrl> sources,
                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> urls);
    bool blockPaste(quint64 winId, const QList<QUrl> &fromUrls, const QUrl &to);
    bool disableOpenWidgetWidget(const QUrl &url, bool *result);
    bool handleCanTag(const QUrl &url, bool *canTag);
    bool handleIsSubFile(const QUrl &parent, const QUrl &sub);
    bool handleNotCdComputer(const QUrl &url, QUrl *cdUrl);

private:
    explicit TrashFileHelper(QObject *parent = nullptr);
};
DPTRASH_END_NAMESPACE
Q_DECLARE_METATYPE(QUrl *);
#endif   // TRASHFILEHELPER_H
