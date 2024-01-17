// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/framelogmanager_p.h"

#include <dfm-framework/log/filterappender.h>

#include <mutex>

Q_LOGGING_CATEGORY(logDPF, "org.deepin.dde.filemanager.lib.framework")

DCORE_USE_NAMESPACE
DPF_USE_NAMESPACE

FrameLogManagerPrivate::FrameLogManagerPrivate(FrameLogManager *qq)
    : q(qq)
{
}

void FrameLogManagerPrivate::initFilterAppender()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        curFilterAppender = new FilterAppender(DTK_CORE_NAMESPACE::DLogManager::getlogFilePath());
        curFilterAppender->setFormat(
                "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n");
        curFilterAppender->setLogFilesLimit(5);
        curFilterAppender->setDatePattern(FilterAppender::kDailyRollover);
        loggerInstance()->registerAppender(curFilterAppender);
    });
}

FilterAppender *FrameLogManagerPrivate::filterAppender()
{
    if (!curFilterAppender)
        initFilterAppender();
    return curFilterAppender;
}

/*!
 * \class The FrameLogManager class
 * re-wrap the log function of dtk, add filter function
 */

FrameLogManager *FrameLogManager::instance()
{
    static FrameLogManager ins;
    return &ins;
}

void FrameLogManager::applySuggestedLogSettings()
{
// DtkCore 5.6.8版本，支持journal方式日志存储，
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    DLogManager::registerJournalAppender();   // 开启journal日志存储
#    ifdef QT_DEBUG
    DLogManager::registerConsoleAppender();   // Release下，标准输出需要禁止，否则会导致一系列问题
#    endif
// 为保证兼容性，在该版本以下，采用原有log文件日志输出方式保存日志
#else
    DLogManager::registerConsoleAppender();
    d->initFilterAppender();
#endif
}

Dtk::Core::Logger *FrameLogManager::globalDtkLogger()
{
    return DTK_CORE_NAMESPACE::Logger::globalInstance();
}

FrameLogManager::FrameLogManager(QObject *parent)
    : QObject(parent),
      d(new FrameLogManagerPrivate(this))
{
}

FrameLogManager::~FrameLogManager()
{
}
