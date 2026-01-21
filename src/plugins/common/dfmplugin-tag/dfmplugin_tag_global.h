// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_TAG_GLOBAL_H
#define DFMPLUGIN_TAG_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DPTAG_NAMESPACE dfmplugin_tag

#define DPTAG_BEGIN_NAMESPACE namespace DPTAG_NAMESPACE {
#define DPTAG_END_NAMESPACE }
#define DPTAG_USE_NAMESPACE using namespace DPTAG_NAMESPACE;

#include <QObject>

DPTAG_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DPTAG_NAMESPACE)

enum class QueryOpts : int {
    kTags,   // get all tags
    kFilesWithTags,   // get all files With tags
    kTagsOfFile,   // get tags of a file
    kFilesOfTag,   // get files of a tag
    kColorOfTags,   // get color-tag map
    kTagIntersectionOfFiles,   // get tag intersection of files
    kTrashFileTags,   // get trash file tags
    kAllTrashFileTags   // get all trash file tags
};

enum class InsertOpts : int {
    kTags,
    kTagOfFiles,
    kTrashFileTags   // save trash file tags
};

enum class DeleteOpts : int {
    kTags,
    kFiles,
    kTagOfFiles,
    kTrashFileTags,   // delete trash file tags
    kAllTrashTags   // clear all trash file tags
};

enum class UpdateOpts : int {
    kColors,
    kTagsNameWithFiles,
    kFilesPaths
};

inline constexpr int kTagDiameter { 10 };

namespace TagActionId {
inline constexpr char kActTagColorListKey[] { "tag-color-list" };
inline constexpr char kActTagAddKey[] { "tag-add" };
inline constexpr char kOpenFileLocation[] { "open-file-location" };
}

namespace AcName {
inline constexpr char kAcSidebarTagitemMenu[] { "sidebar_tagitem_menu" };
inline constexpr char kAcTagWidget[] { "tag_widget" };
inline constexpr char kAcTagLable[] { "tag_lable" };
inline constexpr char kAcTagLeftLable[] { "tag_left_lable" };
inline constexpr char kAcTagCrumbEdit[] { "tag_crumb_edit" };
}

DPTAG_END_NAMESPACE

#endif   // DFMPLUGIN_TAG_GLOBAL_H
