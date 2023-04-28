// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmmimedata.h"
#include "private/dfmmimedata_p.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/urlroute.h>

#include <QJsonDocument>

using namespace dfmbase;

inline constexpr char kVersion[] { "1.0" };

inline constexpr char kUrlsKey[] { "urls" };
inline constexpr char kVersionKey[] { "version" };

// attritubes
inline constexpr char kCanTrashAttr[] { "canTrash" };
inline constexpr char kCanDeleteAttr[] { "canDelete" };

DFMMimeDataPrivate::DFMMimeDataPrivate()
    : QSharedData(),
      version(kVersion)
{
}

DFMMimeDataPrivate::DFMMimeDataPrivate(const DFMMimeDataPrivate &other)
    : QSharedData(other),
      version(other.version)
{
}

DFMMimeDataPrivate::~DFMMimeDataPrivate()
{
}

void DFMMimeDataPrivate::parseUrls(const QList<QUrl> &urls)
{
    urlList = urls;
    bool canTrash = true;
    bool canDelete = true;

    for (const auto &url : urls) {
        auto info = InfoFactory::create<FileInfo>(url);
        if (canTrash && !info->canAttributes(FileInfo::FileCanType::kCanTrash))
            canTrash = false;
        if (canDelete && !info->canAttributes(FileInfo::FileCanType::kCanDelete))
            canDelete = false;

        if (!canTrash && !canDelete)
            break;
    }

    attributes.insert(kCanTrashAttr, canTrash);
    attributes.insert(kCanDeleteAttr, canDelete);
}

DFMMimeData::DFMMimeData()
    : d(new DFMMimeDataPrivate)
{
}

DFMMimeData::DFMMimeData(const DFMMimeData &other)
    : d(other.d)
{
}

DFMMimeData::~DFMMimeData()
{
}

void DFMMimeData::setUrls(const QList<QUrl> &urls)
{
    d->parseUrls(urls);
}

QList<QUrl> DFMMimeData::urls() const
{
    return d->urlList;
}

bool DFMMimeData::canTrash() const
{
    return attritube(kCanTrashAttr, false).toBool();
}

bool DFMMimeData::canDelete() const
{
    return attritube(kCanDeleteAttr, false).toBool();
}

QString DFMMimeData::version() const
{
    return d->version;
}

QByteArray DFMMimeData::toByteArray()
{
    if (d->urlList.isEmpty())
        return {};

    QVariantMap data;
    data.insert(kVersionKey, d->version);
    data.insert(kUrlsKey, QUrl::toStringList(d->urlList));
    data.unite(d->attributes);

    QJsonDocument doc = QJsonDocument::fromVariant(data);
    return doc.toJson();
}

void DFMMimeData::clear()
{
    d->urlList.clear();
    d->attributes.clear();
    d->version = kVersion;
}

void DFMMimeData::setAttritube(const QString &name, const QVariant &value)
{
    if (d->attributes.contains(name))
        return;

    d->attributes.insert(name, value);
}

QVariant DFMMimeData::attritube(const QString &name, const QVariant &defaultValue) const
{
    return d->attributes.value(name, defaultValue);
}

bool DFMMimeData::isValid() const
{
    return !d->urlList.isEmpty();
}

DFMMimeData DFMMimeData::fromByteArray(const QByteArray &data)
{
    DFMMimeData mimeData;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isEmpty())
        return mimeData;

    QVariantMap map = doc.toVariant().toMap();
    const auto &version = map.take(kVersionKey).toString();
    if (version.isEmpty() || version != kVersion)
        return mimeData;

    mimeData.d->version = version;
    mimeData.d->urlList = QUrl::fromStringList(map.take(kUrlsKey).toStringList());
    mimeData.d->attributes = map;

    return mimeData;
}
