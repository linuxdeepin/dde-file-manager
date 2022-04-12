/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef FRAMELOGMANAGER_H
#define FRAMELOGMANAGER_H

#include "dfm-framework/dfm_framework_global.h"

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
    static FrameLogManager &instance();

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

#endif   // LOGMANAGER_H
