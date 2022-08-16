// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_H
#define DAEMONPLUGIN_H

#include <QObject>

namespace GrandSearch {
class DaemonLibrary;
class DaemonPlugin : public QObject
{
    Q_OBJECT
public:
    explicit DaemonPlugin(QObject *parent = nullptr);
    ~DaemonPlugin() override;
    bool initialize();
public slots:
    bool start();
    void stop();
private:
    DaemonLibrary *library = nullptr;
};
}

#endif // DAEMONPLUGIN_H
