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

#ifndef DFM_DTHUMBNAILPROVIDER_H
#define DFM_DTHUMBNAILPROVIDER_H

#include "dfm_base_global.h"

#include <QThread>
#include <QFileInfo>

#include <functional>

class QMimeType;

namespace dfmbase {

class DThumbnailProviderPrivate;

class DThumbnailProvider : public QThread
{
    Q_OBJECT

public:
    enum Size : uint16_t {
        kSmall = 64,
        kNormal = 128,
        kLarge = 256,
    };

    static DThumbnailProvider *instance();

    bool hasThumbnail(const QFileInfo &info) const;
    bool hasThumbnail(const QMimeType &mimeType) const;

    QString thumbnailFilePath(const QFileInfo &info, Size size) const;

    QString createThumbnail(const QFileInfo &info, Size size);

    using CallBack = std::function<void(const QString &)>;
    void appendToProduceQueue(const QString &filePath, Size size, CallBack callback = nullptr);

    QString errorString() const;

    qint64 sizeLimit(const QMimeType &mimeType) const;

signals:
    void createThumbnailFinished(const QString &sourceFilePath, const QString &thumbnailPath) const;
    void createThumbnailFailed(const QString &sourceFilePath) const;

protected:
    explicit DThumbnailProvider(QObject *parent = nullptr);
    ~DThumbnailProvider() override;

    void run() override;

private:
    QScopedPointer<DThumbnailProviderPrivate> d;
};

}

#endif   // DFM_DTHUMBNAILPROVIDER_H
