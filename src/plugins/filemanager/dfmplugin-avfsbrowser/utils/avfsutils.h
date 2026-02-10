// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AVFSUTILS_H
#define AVFSUTILS_H

#include "dfmplugin_avfsbrowser_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>

#include <QString>
#include <QUrl>

namespace dfmplugin_avfsbrowser {

class AvfsUtils : public QObject
{
    Q_DISABLE_COPY(AvfsUtils)
public:
    static AvfsUtils *instance();

    static inline QString scheme()
    {
        return "avfs";
    }

    static inline QUrl rootUrl()
    {
        QUrl u;
        u.setScheme(scheme());
        u.setPath("/");
        return u;
    }

    static inline QStringList supportedArchives()
    {
        QStringList supported = dfmbase::MimeTypeDisplayManager::instance()->supportArchiveMimetypes();
        supported.removeAll(dfmbase::Global::Mime::kTypeCdImage);
        supported.removeAll(dfmbase::Global::Mime::kTypeArchiveRAR);
        supported.removeAll(dfmbase::Global::Mime::kTypeAppRAR);
        return supported;
    }
    static bool isSupportedArchives(const QUrl &url);
    static bool isSupportedArchives(const QString &path);

    static bool isAvfsMounted();
    static void mountAvfs();
    static void unmountAvfs();
    static QString avfsMountPoint();
    static bool archivePreviewEnabled();

    static QUrl avfsUrlToLocal(const QUrl &avfsUrl);
    static QUrl localUrlToAvfsUrl(const QUrl &url);
    static QUrl localArchiveToAvfsUrl(const QUrl &url);
    static QUrl makeAvfsUrl(const QString &path);

    static QList<QVariantMap> seperateUrl(const QUrl &url);
    static QString parseDirIcon(QString path);

private:
    explicit AvfsUtils(QObject *parent = nullptr);
};

}

#endif   // AVFSUTILS_H
