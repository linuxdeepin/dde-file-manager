// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logutil.h"
#include "log/dfmLogManager.h"
#include "log/filterAppender.h"

DFM_USE_NAMESPACE
LogUtil::LogUtil()
{

}

LogUtil::~LogUtil()
{

}

void LogUtil::registerLogger()
{
    DFMLogManager::registerConsoleAppender();
    DFMLogManager::registerFileAppender();

    DFMLogManager::getLogger()->logToGlobalInstance("file.job", true);
}
