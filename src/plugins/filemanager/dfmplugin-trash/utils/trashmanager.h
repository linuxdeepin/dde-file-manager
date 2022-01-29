/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include "dfmplugin_trash_global.h"

#include "dfm-base/utils/clipboard.h"

#include <QUrl>
#include <QIcon>

DPTRASH_BEGIN_NAMESPACE

class TrashManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TrashManager)

public:
    static TrashManager *instance();

    inline static QString scheme()
    {
        return "trash";
    }

    inline static QIcon icon()
    {
        return QIcon::fromTheme("user-trash-symbolic");
    }

    static QUrl rootUrl();

    static void contenxtMenuHandle(quint64 windowId, const QUrl &url, const QPoint &globalPos);
    static bool openFilesHandle(quint64 windowId, const QList<QUrl> urls, const QString *error);

    static bool writeToClipBoardHandle(const quint64 windowId,
                                       const DFMBASE_NAMESPACE::ClipBoard::ClipboardAction action,
                                       const QList<QUrl> urls);
    static QUrl fromTrashFile(const QString &filePath)
    {
        QUrl url;

        url.setScheme(TrashManager::scheme());
        url.setPath(filePath);

        return url;
    }

    static QUrl toLocalFile(const QUrl &url);

private:
    explicit TrashManager(QObject *parent = nullptr);
    ~TrashManager() override;
    void init();
};

DPTRASH_END_NAMESPACE
#endif   // TRASHMANAGER_H
