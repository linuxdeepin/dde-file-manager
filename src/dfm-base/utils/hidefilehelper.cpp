// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hidefilehelper.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/utils/decorator/decoratorfile.h"

#include "dfm-io/dfmio_register.h"
#include "dfm-io/core/dfile.h"
#include "dfm-io/core/diofactory.h"

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
        dfile = DFMBASE_NAMESPACE::DecoratorFile(fileUrl).filePtr();

        if (!dfile)
            return;

        if (dfile->open(DFMIO::DFile::OpenFlag::kReadOnly)) {
            QByteArray data = dfile->readAll();
            const QString &dataStr = QString::fromLocal8Bit(data);
            hideList = QSet<QString>::fromList(dataStr.split('\n', QString::SkipEmptyParts));
            hideListUpdate = hideList;
            dfile->close();
        }
    }
    void updateAttribute()
    {
        for (const QString &name : hideListUpdate) {
            const QString &path = dirUrl.toLocalFile() + "/" + name;
            const QUrl &url = QUrl::fromLocalFile(path);
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
            info->refresh();

            DFMBASE_NAMESPACE::DecoratorFileInfo decorator(url);
            decorator.notifyAttributeChanged();
        }
        AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(fileUrl);
        info->refresh();
        DFMBASE_NAMESPACE::DecoratorFileInfo decorator(fileUrl);
        // TODO(gongheng): why return false in vault
        decorator.notifyAttributeChanged();
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

    QStringList lines(d->hideList.toList());
    QString dataStr = lines.join('\n');
    QByteArray data;
    data.append(dataStr);

    if (d->dfile->open(DFMIO::DFile::OpenFlag::kWriteOnly)) {
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
