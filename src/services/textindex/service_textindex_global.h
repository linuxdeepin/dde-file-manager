// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_TEXTINDEX_GLOBAL_H
#define SERVICE_TEXTINDEX_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>
#include <dfm-search/dsearch_global.h>

#define SERVICETEXTINDEX_NAMESPACE service_textindex
#define SERVICETEXTINDEX_BEGIN_NAMESPACE namespace SERVICETEXTINDEX_NAMESPACE {
#define SERVICETEXTINDEX_END_NAMESPACE }
#define SERVICETEXTINDEX_USE_NAMESPACE using namespace SERVICETEXTINDEX_NAMESPACE;

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace Defines {

inline const QString kTextIndexServiceName =
        QLatin1String("deepin-service-plugin@org.deepin.Filemanager.TextIndex.service");
inline const QString kAnythingDocType = QLatin1String("doc");

// Dconfig
namespace DConf {
inline const QString kTextIndexSchema = QLatin1String("org.deepin.dde.file-manager.textindex");
inline const QString kAutoIndexUpdateInterval = QLatin1String("autoIndexUpdateInterval");
inline const QString kInotifyResourceCleanupDelay = QLatin1String("inotifyResourceCleanupDelay");
inline const QString kMaxIndexFileSizeMB = QLatin1String("maxIndexFileSizeMB");
inline const QString kSupportedFileExtensions = QLatin1String("supportedFileExtensions");
inline const QString kIndexHiddenFiles = QLatin1String("indexHiddenFiles");
inline const QString kFolderExcludeFilters = QLatin1String("folderExcludeFilters");
}   // namesapce DConf

// NOTE: The version number must be upgraded
// when the index contents are changed to ensure
// that the index can be rebuilt!!!
inline constexpr int kIndexVersion { 0 };

// json
inline const QString kVersionKey = QLatin1String("version");
inline const QString kLastUpdateTimeKey = QLatin1String("lastUpdateTime");
}   // namespace Defines

DFM_LOG_USE_CATEGORY(SERVICETEXTINDEX_NAMESPACE)

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SERVICE_TEXTINDEX_GLOBAL_H
