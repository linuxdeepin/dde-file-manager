// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
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
inline const QString kOcrIndexServiceName =
        QLatin1String("deepin-service-plugin@org.deepin.Filemanager.OcrIndex.service");
inline const QString kAnythingDirType = QLatin1String("dir");
inline const QString kAnythingDocType = QLatin1String("doc");
inline const QString kAnythingPicType = QLatin1String("pic");
inline const QString kTextIndexDBusService = QLatin1String("org.deepin.Filemanager.TextIndex");
inline const QString kTextIndexDBusObjectPath = QLatin1String("/org/deepin/Filemanager/TextIndex");
inline const QString kOcrIndexDBusService = QLatin1String("org.deepin.Filemanager.OcrIndex");
inline const QString kOcrIndexDBusObjectPath = QLatin1String("/org/deepin/Filemanager/OcrIndex");

// Dconfig
namespace DConf {
inline const QString kTextIndexSchema = QLatin1String("org.deepin.dde.file-manager.textindex");
inline const QString kAutoIndexUpdateInterval = QLatin1String("autoIndexUpdateInterval");
inline const QString kMonitoringStartDelaySeconds = QLatin1String("monitoringStartDelaySeconds");
inline const QString kSilentIndexUpdateDelay = QLatin1String("silentIndexUpdateDelay");
inline const QString kInotifyResourceCleanupDelay = QLatin1String("inotifyResourceCleanupDelay");
inline const QString kMaxIndexTextFileSizeMB = QLatin1String("maxIndexFileSizeMB");
inline const QString kMaxIndexFileTruncationSizeMB = QLatin1String("maxIndexFileTruncationSizeMB");
inline const QString kSupportedTextFileExtensions = QLatin1String("supportedFileExtensions");
inline const QString kIndexHiddenFiles = QLatin1String("indexHiddenFiles");
inline const QString kFolderExcludeFilters = QLatin1String("folderExcludeFilters");
inline const QString kCpuUsageLimitPercent = QLatin1String("cpuUsageLimitPercent");
inline const QString kInotifyWatchesCoefficient = QLatin1String("inotifyWatchesCoefficient");
inline const QString kBatchCommitInterval = QLatin1String("batchCommitInterval");

}   // namesapce DConf

// NOTE: The version number must be upgraded
// when the index contents are changed to ensure
// that the index can be rebuilt!!!
// History:
// Version 1: add "filename" filed
// Version 2: add new filed "ancestor_paths"
// Version 3: add new time-related fields
inline constexpr int kTextIndexVersion { 3 };

// OCR index version history:
// Version 0: initial OCR text index schema
inline constexpr int kOcrIndexVersion { 0 };

// json
inline const QString kTextVersionKey = QLatin1String("version");
inline const QString kOcrVersionKey = QLatin1String("version");
inline const QString kLastUpdateTimeKey = QLatin1String("lastUpdateTime");
inline const QString kStateKey = QLatin1String("state");
inline const QString kStateClean = QLatin1String("clean");
inline const QString kStateDirty = QLatin1String("dirty");
inline const QString kNeedsRebuildKey = QLatin1String("needsRebuild");
}   // namespace Defines

DFM_LOG_USE_CATEGORY(SERVICETEXTINDEX_NAMESPACE)

SERVICETEXTINDEX_END_NAMESPACE

#endif   // SERVICE_TEXTINDEX_GLOBAL_H
