// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALFILEHELPER_H
#define OPTICALFILEHELPER_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QObject>

DPOPTICAL_BEGIN_NAMESPACE

class OpticalFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OpticalFileHelper)
public:
    static OpticalFileHelper *instance();
    inline static QString scheme()
    {
        return DFMGLOBAL_NAMESPACE::Scheme::kBurn;
    }

    bool cutFile(const quint64 windowId, const QList<QUrl> sources,
                 const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool copyFile(const quint64 windowId, const QList<QUrl> sources,
                  const QUrl target, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool moveToTrash(const quint64 windowId, const QList<QUrl> sources,
                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool openFileInPlugin(quint64 winId, QList<QUrl> urls);
    bool writeUrlsToClipboard(const quint64 windowId, const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                              const QList<QUrl> urls);
    bool openFileInTerminal(const quint64 windowId, const QList<QUrl> urls);

private:
    void pasteFilesHandle(const QList<QUrl> sources, const QUrl target, bool isCopy = true);

private:
    explicit OpticalFileHelper(QObject *parent = nullptr);
};
DPOPTICAL_END_NAMESPACE

#endif   // OPTICALFILEHELPER_H
