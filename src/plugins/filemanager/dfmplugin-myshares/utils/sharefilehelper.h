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
#ifndef SHAREFILEHELPER_H
#define SHAREFILEHELPER_H

#include "dfmplugin_myshares_global.h"

#include <QObject>

DPMYSHARES_BEGIN_NAMESPACE

class ShareFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ShareFileHelper)
public:
    static ShareFileHelper *instance();
    inline static QString scheme()
    {
        return "usershare";
    }

    // file operation
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> urls);

private:
    explicit ShareFileHelper(QObject *parent = nullptr);
};
DPMYSHARES_END_NAMESPACE

#endif   // SHAREFILEHELPER_H
