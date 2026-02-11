// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGFILEWATCHER_H
#define TAGFILEWATCHER_H

#include "dfmplugin_tag_global.h"
#include <dfm-base/interfaces/abstractfilewatcher.h>

namespace dfmplugin_tag {

class TagFileWatcherPrivate;
class TagFileWatcher : public DFMBASE_NAMESPACE::AbstractFileWatcher
{
    Q_OBJECT
public:
    explicit TagFileWatcher() = delete;
    explicit TagFileWatcher(const QUrl &url, QObject *parent = nullptr);
    ~TagFileWatcher() override;

    virtual void setEnabledSubfileWatcher(const QUrl &subfileUrl, bool enabled = true) override;

public slots:
    void onTagRemoved(const QString &tagName);
    void onFilesTagged(const QVariantMap &fileAndTags);
    void onFilesUntagged(const QVariantMap &fileAndTags);
    void onFilesHidden(const QVariantMap &fileAndTags);

private:
    TagFileWatcherPrivate *dptr = nullptr;
};

}

#endif   // TAGFILEWATCHER_H
