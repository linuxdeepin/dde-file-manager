// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class WindowManager;
class AppController;
class DUrl;
class QFileSystemWatcher;

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

class FileManagerApp : public QObject
{
    Q_OBJECT

public:
    static FileManagerApp *instance();

    ~FileManagerApp();

    void initManager();
    void initTranslation();
    static void initService();

private:
    void initApp();
    void initView();
    void lazyRunTask();
    void initSysPathWatcher();
    void initConnect();

//    QString getFileJobConfigPath();

public slots:
    void show(const DUrl &url);
    void showPropertyDialog(const QStringList paths);
    void openWithDialog(const QStringList files);

protected:
    explicit FileManagerApp(QObject *parent = nullptr);

private:
    WindowManager *m_windowManager = nullptr;
    QFileSystemWatcher *m_sysPathWatcher;
};

#endif // FILEMANAGERAPP_H
