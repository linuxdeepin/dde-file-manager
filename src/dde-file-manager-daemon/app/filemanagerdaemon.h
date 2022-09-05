// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERDAEMON_H
#define FILEMANAGERDAEMON_H

#include <QObject>

class AppController;

class FileManagerDaemon : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerDaemon(QObject *parent = nullptr);
    ~FileManagerDaemon();

    void initControllers();
    void initConnect();

signals:

public slots:

private:
    AppController* m_appController = nullptr;
};

#endif // FILEMANAGERDAEMON_H
