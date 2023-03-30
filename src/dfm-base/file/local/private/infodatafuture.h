// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INFODATAFUTURE_H
#define INFODATAFUTURE_H

#include <dfm-io/dfilefuture.h>

#include <QObject>
#include <QMap>
#include <QVariant>

USING_IO_NAMESPACE
namespace dfmbase {
class InfoDataFuture : public QObject
{
    Q_OBJECT
public:
    ~InfoDataFuture() override;

Q_SIGNALS:
    void infoMediaAttributes(const QUrl &url, const QMap<DFileInfo::AttributeExtendID, QVariant> &map);

private Q_SLOTS:
    void infoMedia(const QUrl &url, const QMap<DFileInfo::AttributeExtendID, QVariant> &map);

private:
    explicit InfoDataFuture(DFileFuture *future, QObject *parent = nullptr);
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> mediaInfo() const;
    bool isFinished() const;
    friend class AsyncFileInfoPrivate;
    friend class SyncFileInfoPrivate;

private:
    QSharedPointer<DFileFuture> future { nullptr };
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> attribute;
    std::atomic_bool finshed { false };
};
}

#endif   // INFODATAFUTURE_H
