// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWHELPER_H
#define PREVIEWHELPER_H

#include <dfm-base/utils/clipboard.h>

namespace dfmplugin_filepreview {
class PreviewHelper : public QObject
{
    Q_DISABLE_COPY(PreviewHelper)
public:
    static PreviewHelper *instance();

    bool isPreviewEnabled();
    bool showThumbnailInRemote();
    bool showThumbnailInMtp();

    void bindConfig();

private:
    explicit PreviewHelper(QObject *parent = nullptr);

    static void saveRemoteToConf(const QVariant &var);
    static void syncRemoteToAppSet(const QString &, const QString &, const QVariant &var);
    static bool isRemoteConfEqual(const QVariant &dcon, const QVariant &dset);
};
}

#endif   // PREVIEWHELPER_H
