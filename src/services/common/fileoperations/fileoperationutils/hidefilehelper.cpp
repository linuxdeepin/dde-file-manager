/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
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

#include "hidefilehelper.h"

#include "dfm-io/dfmio_register.h"
#include "dfm-io/core/dfile.h"
#include "dfm-io/core/diofactory.h"

#include <QSet>
#include <QFile>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE
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
        QSharedPointer<DFMIO::DIOFactory> factory = produceQSharedIOFactory(fileUrl.scheme(), static_cast<QUrl>(fileUrl));
        if (!factory) {
            return;
        }
        dfile = factory->createFile();
        if (!dfile)
            return;

        if (dfile->open(DFMIO::DFile::OpenFlag::ReadOnly)) {
            QByteArray data = dfile->readAll();
            const QString &dataStr = QString::fromLocal8Bit(data);
            hideList = QSet<QString>::fromList(dataStr.split('\n', QString::SkipEmptyParts));
            dfile->close();
        }
    }

public:
    HideFileHelper *q = nullptr;
    QUrl dirUrl;
    QUrl fileUrl;
    QSet<QString> hideList;
    QSharedPointer<DFMIO::DFile> dfile = nullptr;
};

DFMBASE_END_NAMESPACE

DFMBASE_USE_NAMESPACE
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

    if (d->dfile->open(DFMIO::DFile::OpenFlag::WriteOnly)) {
        d->dfile->write(data);
        d->dfile->close();
    }
}

bool HideFileHelper::insert(const QString &name)
{
    d->hideList.insert(name);
}

bool HideFileHelper::remove(const QString &name)
{
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
