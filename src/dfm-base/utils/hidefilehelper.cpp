// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hidefilehelper.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include <dfm-io/dfile.h>
#include <dfm-io/dfileinfo.h>

#include <QSet>
#include <QFile>
#include <QDebug>

namespace dfmbase {
class HideFileHelperPrivate
{
public:
    HideFileHelperPrivate(HideFileHelper *qptr, const QUrl &dir)
        : q(qptr)
    {
        dirUrl = dir;

        QString dirStr = dirUrl.toString();
        if (!dirStr.endsWith("/"))
            dirStr.append("/");
        fileUrl = dirStr.append(".hidden");

        init();
    }
    ~HideFileHelperPrivate() = default;

    void init()
    {
        dfile.reset(new DFMIO::DFile(fileUrl));

        if (!dfile)
            return;

        if (dfile->open(DFMIO::DFile::OpenFlag::kReadOnly)) {
            const QByteArray &data = dfile->readAll();
            const QString &dataStr = QString::fromLocal8Bit(data);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            const QStringList &splitList = dataStr.split('\n', Qt::SkipEmptyParts);
            hideList = QSet<QString>(splitList.begin(), splitList.end());
#else
            hideList = QSet<QString>::fromList(dataStr.split('\n', QString::SkipEmptyParts));
#endif
            hideListUpdate = hideList;
            dfile->close();
        }
    }
    void updateAttribute()
    {
        for (const QString &name : hideListUpdate) {
            const QString &path = dirUrl.toLocalFile() + "/" + name;
            const QUrl &url = QUrl::fromLocalFile(path);
            FileInfoPointer info = InfoFactory::create<FileInfo>(url);
            info->refresh();

            DFMIO::DFileInfo dfileInfo { url };
            dfileInfo.setCustomAttribute("xattr::update",
                                         DFMIO::DFileInfo::DFileAttributeType::kTypeString, "");
        }
        FileInfoPointer info = InfoFactory::create<FileInfo>(fileUrl);
        info->refresh();
        // TODO(gongheng): why return false in vault
        DFMIO::DFileInfo dfileInfo { fileUrl };
        dfileInfo.setCustomAttribute("xattr::update",
                                     DFMIO::DFileInfo::DFileAttributeType::kTypeString, "");
    }

public:
    HideFileHelper *q = nullptr;
    QUrl dirUrl;
    QUrl fileUrl;
    QSet<QString> hideList;
    QSet<QString> hideListUpdate;
    QSharedPointer<DFMIO::DFile> dfile = nullptr;
};

}

using namespace dfmbase;
HideFileHelper::HideFileHelper(const QUrl &dir)
    : d(new HideFileHelperPrivate(this, dir))
{
}

HideFileHelper::~HideFileHelper()
{
}

QUrl HideFileHelper::dirUrl() const
{
    return d->dirUrl;
}

QUrl HideFileHelper::fileUrl() const
{
    return d->fileUrl;
}

bool HideFileHelper::save() const
{
    if (!d->dfile)
        return false;

    QStringList lines(d->hideList.values());
    QString dataStr = lines.join('\n');
    QByteArray data;
    data.append(dataStr.toUtf8());

    if (d->dfile->open(DFMIO::DFile::OpenFlag::kWriteOnly | DFMIO::DFile::OpenFlag::kTruncate)) {
        d->dfile->write(data);
        d->dfile->close();
        d->updateAttribute();
        return true;
    }
    return false;
}

bool HideFileHelper::insert(const QString &name)
{
    d->hideList.insert(name);
    d->hideListUpdate.insert(name);

    return true;
}

bool HideFileHelper::remove(const QString &name)
{
    d->hideListUpdate.insert(name);
    return d->hideList.remove(name);
}

bool HideFileHelper::contains(const QString &name)
{
    return d->hideList.contains(name);
}

QSet<QString> HideFileHelper::hideFileList() const
{
    return d->hideList;
}
