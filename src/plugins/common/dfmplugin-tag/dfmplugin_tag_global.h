// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMPLUGIN_TAG_GLOBAL_H
#define DFMPLUGIN_TAG_GLOBAL_H

#define DPTAG_NAMESPACE dfmplugin_tag

#define DPTAG_BEGIN_NAMESPACE namespace DPTAG_NAMESPACE {
#define DPTAG_END_NAMESPACE }
#define DPTAG_USE_NAMESPACE using namespace DPTAG_NAMESPACE;

#include <QObject>

DPTAG_BEGIN_NAMESPACE

enum class TagActionType : uint32_t {
    kGetAllTags = 1,
    kGetTagsThroughFile,
    kGetSameTagsOfDiffFiles,
    kGetFilesThroughTag,
    kGetTagsColor,
    kAddTags,
    kMakeFilesTags,
    kMakeFilesTagThroughColor,
    kRemoveTagsOfFiles,
    kDeleteTags,
    kDeleteFiles,
    kChangeTagsColor,
    kChangeTagsNameWithFiles,
    kChangeFilesPaths
};

inline constexpr int kTagDiameter { 10 };

namespace TagActionId {
inline constexpr char kActTagColorListKey[] { "tag-color-list" };
inline constexpr char kActTagAddKey[] { "tag-add" };
inline constexpr char kOpenFileLocation[] { "open-file-location" };
}

DPTAG_END_NAMESPACE

#endif   // DFMPLUGIN_TAG_GLOBAL_H
