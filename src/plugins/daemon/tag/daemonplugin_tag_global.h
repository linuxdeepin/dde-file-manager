// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DAEMONPLUGIN_TAG_GLOBAL_H
#define DAEMONPLUGIN_TAG_GLOBAL_H

#include <dfm-base/dfm_log_defines.h>

#define DAEMONPTAG_NAMESPACE daemonplugin_tag

#define DAEMONPTAG_BEGIN_NAMESPACE namespace DAEMONPTAG_NAMESPACE {
#define DAEMONPTAG_END_NAMESPACE }
#define DAEMONPTAG_USE_NAMESPACE using namespace DAEMONPTAG_NAMESPACE;

DAEMONPTAG_BEGIN_NAMESPACE
DFM_LOG_USE_CATEGORY(DAEMONPTAG_NAMESPACE)

// TODO: refactor it, make atomization interfaces

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
    kTrashFileTags,   // remove trash file tags
    kAllTrashTags   // clear all trash tags
};

enum class UpdateOpts : int {
    kColors,
    kTagsNameWithFiles,
    kFilesPaths
};

DAEMONPTAG_END_NAMESPACE

#endif   // DAEMONPLUGIN_TAG_GLOBAL_H
