/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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
#ifndef OPTICALFILEHELPER_H
#define OPTICALFILEHELPER_H

#include "dfmplugin_optical_global.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/utils/clipboard.h"

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
    bool linkFile(const quint64 windowId, const QUrl url, const QUrl link, const bool force, const bool silence);
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
