// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERCLIENT_H
#define FILEMANAGERCLIENT_H

#include <QObject>

#include "dbusservice/dbusinterface/fileoperation_interface.h"

class FileManagerClient : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerClient(QObject *parent = nullptr);
    ~FileManagerClient();

signals:

public slots:
    void testNewNameJob(const QString &oldFile, const QString &newFile);

private:
    FileOperationInterface* m_fileOperationInterface = nullptr;

};

#endif // FILEMANAGERCLIENT_H
