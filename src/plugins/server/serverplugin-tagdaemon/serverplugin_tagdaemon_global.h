// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVERPLUGIN_TAGDAEMON_GLOBAL_H
#define SERVERPLUGIN_TAGDAEMON_GLOBAL_H

#define SERVERTAGDAEMON_NAMESPACE serverplugin_tagdaemon

#define SERVERTAGDAEMON_BEGIN_NAMESPACE namespace SERVERTAGDAEMON_NAMESPACE {
#define SERVERTAGDAEMON_END_NAMESPACE }
#define SERVERTAGDAEMON_USE_NAMESPACE using namespace SERVERTAGDAEMON_NAMESPACE;

SERVERTAGDAEMON_BEGIN_NAMESPACE

// TODO: refactor it, make atomization interfaces

enum class QueryOpts : int {
    kTags,   // get all tags
    kFilesWithTags,   // get all files With tags
    kTagsOfFile,   // get tags of a file
    kFilesOfTag,   // get files of a tag
    kColorOfTags,   // get color-tag map
    kTagIntersectionOfFiles   // get tag intersection of files
};

enum class InsertOpts : int {
    kTags,
    kTagOfFiles
};

enum class DeleteOpts : int {
    kTags,
    kFiles,
    kTagOfFiles
};

enum class UpdateOpts : int {
    kColors,
    kTagsNameWithFiles,
    kFilesPaths
};

SERVERTAGDAEMON_END_NAMESPACE

#endif   // SERVERPLUGIN_TAGDAEMON_GLOBAL_H
