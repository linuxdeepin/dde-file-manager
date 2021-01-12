/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
    void lazyRunInitServiceTask();
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
