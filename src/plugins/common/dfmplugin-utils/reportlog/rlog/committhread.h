// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMITTHREAD_H
#define COMMITTHREAD_H

#include <QObject>
#include <QLibrary>
namespace dfmplugin_utils {

class CommitLog : public QObject
{
    Q_OBJECT
public:
    using InitEventLog = bool (*)(const std::string &, bool);
    using WriteEventLog = void (*)(const std::string &);

    explicit CommitLog(QObject *parent = nullptr);
    ~CommitLog();
public slots:
    void commit(const QVariant &args);
    bool init();

private:
    QLibrary logLibrary;
    InitEventLog initEventLogFunc = nullptr;
    WriteEventLog writeEventLogFunc = nullptr;
};

}
#endif   // COMMITTHREAD_H
