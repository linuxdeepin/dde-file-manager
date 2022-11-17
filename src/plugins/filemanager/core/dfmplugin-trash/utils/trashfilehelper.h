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
#ifndef TRASHFILEHELPER_H
#define TRASHFILEHELPER_H

#include "dfmplugin_trash_global.h"

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm-base/utils/clipboard.h"

#include <QObject>

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
    bool copyFromFile(const quint64 windowId, const QList<QUrl> sources, const QUrl target,
                      const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool moveToTrash(const quint64 windowId, const QList<QUrl> sources,
                     const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool deleteFile(const quint64 windowId, const QList<QUrl> sources,
                    const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags);
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> urls);

private:
    explicit TrashFileHelper(QObject *parent = nullptr);
};
DPTRASH_END_NAMESPACE

#endif   // TRASHFILEHELPER_H
