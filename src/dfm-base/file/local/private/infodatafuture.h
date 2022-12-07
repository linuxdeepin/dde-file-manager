/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef INFODATAFUTURE_H
#define INFODATAFUTURE_H

#include <dfm-io/core/dfilefuture.h>

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
    friend class LocalFileInfoPrivate;

private:
    QSharedPointer<DFileFuture> future { nullptr };
    QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> attribute;
    std::atomic_bool finshed { false };
};
}

#endif   // INFODATAFUTURE_H
