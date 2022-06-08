/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#ifndef TAGEVENTRECEIVER_H
#define TAGEVENTRECEIVER_H

#include "dfmplugin_tag_global.h"

DPTAG_BEGIN_NAMESPACE

class TagEventReceiver : public QObject
{
    Q_OBJECT
public:
    static TagEventReceiver *instance();
    void initConnect();

public slots:
    void handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg);
    void handleFileRemoveResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg);

private:
    explicit TagEventReceiver(QObject *parent = nullptr);
};

DPTAG_END_NAMESPACE

#endif   // TAGEVENTRECEIVER_H
