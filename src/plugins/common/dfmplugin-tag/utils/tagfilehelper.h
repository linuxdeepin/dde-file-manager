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
#ifndef TAGFILEHELPER_H
#define TAGFILEHELPER_H

#include "dfmplugin_tag_global.h"

#include <QObject>

DPTAG_BEGIN_NAMESPACE

class TagFileHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TagFileHelper)
public:
    static TagFileHelper *instance();
    inline static QString scheme()
    {
        return "tag";
    }

    // file operation
    bool openFileInPlugin(quint64 windowId, const QList<QUrl> urls);

private:
    explicit TagFileHelper(QObject *parent = nullptr);
};
DPTAG_END_NAMESPACE

#endif   // TAGFILEHELPER_H
