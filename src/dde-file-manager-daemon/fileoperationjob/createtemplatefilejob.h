// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CREATETEMPLATEFILEJOB_H
#define CREATETEMPLATEFILEJOB_H

#include <QObject>
#include "basejob.h"

class CreateTemplateFileAdaptor;

class CreateTemplateFileJob : public BaseJob
{
    Q_OBJECT
public:
    explicit CreateTemplateFileJob(const QString &templateFile, const QString &targetDir, QObject *parent = nullptr);
    ~CreateTemplateFileJob();

    static QString BaseObjectPath;
    static QString PolicyKitActionId;
    static int JobId;

signals:
    void Done(const QString& message);

public slots:
    void Execute();

private:
    QString m_templateFile;
    QString m_targetDir;
    int m_jobId = 0;

    CreateTemplateFileAdaptor* m_adaptor;
};

#endif // CREATETEMPLATEFILEJOB_H
