/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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
#ifndef BOOKMARKEVENTRECEIVER_H
#define BOOKMARKEVENTRECEIVER_H

#include "dfmplugin_bookmark_global.h"

#include <QObject>

DPBOOKMARK_BEGIN_NAMESPACE

class BookMarkEventReceiver : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BookMarkEventReceiver)

public:
    static BookMarkEventReceiver *instance();

public slots:
    void handleRenameFile(quint64 windowId, const QUrl &oldUrl, const QUrl &newUrl);
    void handleAddSchemeOfBookMarkDisabled(const QString &scheme);

private:
    explicit BookMarkEventReceiver(QObject *parent = nullptr);
};

DPBOOKMARK_END_NAMESPACE

#endif // BOOKMARKEVENTRECEIVER_H
