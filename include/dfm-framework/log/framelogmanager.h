// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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

class FrameLogManagerPrivate;
class FrameLogManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(FrameLogManager)

public:
    static FrameLogManager *instance();
    void applySuggestedLogSettings();
    Dtk::Core::Logger *globalDtkLogger();

private:
    explicit FrameLogManager(QObject *parent = nullptr);
    ~FrameLogManager();

private:
    QScopedPointer<FrameLogManagerPrivate> d;
};

DPF_END_NAMESPACE

#define dpfLogManager ::DPF_NAMESPACE::FrameLogManager::instance()

#endif   // LOGMANAGER_H
