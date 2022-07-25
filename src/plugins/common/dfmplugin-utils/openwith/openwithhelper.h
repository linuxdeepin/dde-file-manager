/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef OPENWITHHELPER_H
#define OPENWITHHELPER_H
#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_utils {
class OpenWithHelper : public QObject
{
    Q_OBJECT
public:
    explicit OpenWithHelper(QObject *parent = nullptr);

    static QWidget *createOpenWithWidget(const QUrl &url);

signals:

public slots:
};
}
#endif   // OPENWITHHELPER_H
