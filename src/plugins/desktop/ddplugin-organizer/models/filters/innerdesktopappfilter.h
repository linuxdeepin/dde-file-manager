/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef INNERDESKTOPAPPFILTER_H
#define INNERDESKTOPAPPFILTER_H

#include "models/modeldatahandler.h"

#include <QMap>
#include <QList>

class QGSettings;

namespace ddplugin_organizer {

class InnerDesktopAppFilter : public QObject, public ModelDataHandler
{
    Q_OBJECT
public:
    explicit InnerDesktopAppFilter(QObject *parent = nullptr);
    void update();
    void refreshModel();
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
public slots:
    void changed(const QString &key);
protected:
    QGSettings *gsettings = nullptr;
    QMap<QString, QUrl> keys;
    QMap<QString, bool> hidden;
};

}

#endif // INNERDESKTOPAPPFILTER_H
