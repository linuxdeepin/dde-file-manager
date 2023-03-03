// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAMELOGMANAGER_H
#define FRAMELOGMANAGER_H

#include <dfm-framework/dfm_framework_global.h>

#include <QObject>
#include <QScopedPointer>

namespace Dtk {
namespace Core {
class Logger;
}
}

DPF_BEGIN_NAMESPACE

class FilterAppender;
class FrameLogManagerPrivate;
class FrameLogManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FrameLogManager)

public:
    static FrameLogManager *instance();

    void registerConsoleAppender();
    void registerFileAppender();
    QString logFilePath();
    void setlogFilePath(const QString &logFilePath);
    void setLogFormat(const QString &format);
    Dtk::Core::Logger *dtkLogger();
    FilterAppender *filterAppender();

private:
    explicit FrameLogManager(QObject *parent = nullptr);
    ~FrameLogManager();

private:
    QScopedPointer<FrameLogManagerPrivate> d;
};

DPF_END_NAMESPACE

#define dpfLogManager ::DPF_NAMESPACE::FrameLogManager::instance()

#endif   // LOGMANAGER_H
