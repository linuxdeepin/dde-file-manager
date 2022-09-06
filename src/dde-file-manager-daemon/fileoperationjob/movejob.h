// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVEJOB_H
#define MOVEJOB_H

#include <QObject>
#include "basejob.h"

class MoveJobAdaptor;

class MoveJob : public BaseJob
{
    Q_OBJECT
public:
    explicit MoveJob(const QStringList &filelist, const QString &targetDir, QObject *parent = nullptr);
    ~MoveJob();

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

    MoveJobAdaptor* m_adaptor;
};

#endif // MOVEJOB_H
