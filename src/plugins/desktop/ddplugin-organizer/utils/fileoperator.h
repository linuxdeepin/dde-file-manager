/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef FILEOPERATOR_H
#define FILEOPERATOR_H

#include "ddplugin_organizer_global.h"

#include "dfm-base/dfm_global_defines.h"

#include <QObject>
#include <QSharedPointer>

namespace ddplugin_organizer {

class FileOperatorPrivate;
class FileOperator : public QObject
{
    Q_OBJECT
    friend class FileOperatorPrivate;
public:
    ~FileOperator();
    static FileOperator *instance();

    void dropFiles(const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls);
    void dropToTrash(const QList<QUrl> &urls);
    void dropToApp(const QList<QUrl> &urls, const QString &app);
protected:
    explicit FileOperator(QObject *parent = nullptr);

private:
    QSharedPointer<FileOperatorPrivate> d = nullptr;
};

#define FileOperatorIns FileOperator::instance()

}

#endif // FILEOPERATOR_H
