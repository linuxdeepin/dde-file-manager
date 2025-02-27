// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGEVENTRECEIVER_H
#define TAGEVENTRECEIVER_H

#include "dfmplugin_tag_global.h"

namespace dfmplugin_tag {

inline constexpr char kSidebarOrder[] { "SideBar/ItemOrder" };
inline constexpr char kTagOrderKey[] { "tag" };

class TagEventReceiver : public QObject
{
    Q_OBJECT
public:
    static TagEventReceiver *instance();

public slots:
    void handleHideFilesResult(const quint64 &winId, const QList<QUrl> &urls, bool ok);
    void handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg);
    void handleFileRemoveResult(const QList<QUrl> &srcUrls, bool ok, const QString &errMsg);
    void handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg);
    void handleWindowUrlChanged(quint64 winId, const QUrl &url);
    void handleRestoreFromTrashResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls,
                                      const QVariantList &customInfos, bool ok, const QString &errMsg);
    QStringList handleGetTags(const QUrl &url);
    void handleSidebarOrderChanged(quint64 winId, const QString &group, QList<QUrl> urls);

private:
    explicit TagEventReceiver(QObject *parent = nullptr);
};

}

#endif   // TAGEVENTRECEIVER_H
