/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#ifndef COPYJOB_H
#define COPYJOB_H

#include <QObject>
#include "basejob.h"

class CopyJobAdaptor;

class CopyJob : public BaseJob
{
    Q_OBJECT
public:
    explicit CopyJob(const QStringList &filelist, const QString &targetDir, QObject *parent = nullptr);
    ~CopyJob();

    static QString BaseObjectPath;
    static QString PolicyKitActionId;
    static int JobId;

signals:
    void Done(const QString& message);

public slots:
    void Execute();

private:
    QStringList m_filelist;
    QString m_targetDir;
    int m_jobId = 0;

    CopyJobAdaptor* m_adaptor;
};

#endif // COPYJOB_H
