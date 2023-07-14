// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagproxyhandle.h"
#include "private/tagproxyhandle_p.h"
#include "utils/tagmanager.h"

using namespace dfmplugin_tag;
static constexpr char kTagService[] { "org.deepin.filemanager.server" };
static constexpr char kTagDBusPath[] { "/org/deepin/filemanager/server/TagManager" };

TagProxyHandlePrivate::TagProxyHandlePrivate(TagProxyHandle *qq, QObject *parent)
    : QObject(parent),
      q(qq)
{
}

TagProxyHandlePrivate::~TagProxyHandlePrivate()
{
}

bool TagProxyHandlePrivate::isDBusRuning()
{
    return tagDBusInterface && tagDBusInterface->isValid();
}

void TagProxyHandlePrivate::initConnection()
{
    dbusWatcher.reset(new QDBusServiceWatcher(kTagService, QDBusConnection::sessionBus()));
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceRegistered, q, [this] {
        qInfo() << "serviceRegistered: " << kTagService;
        connectToDBus();
    });
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceUnregistered, q, [] {
        qWarning() << "Lost connection: " << kTagService;
    });

    connectToDBus();
}

void TagProxyHandlePrivate::connectToDBus()
{
    disconnCurrentConnections();
    auto ptr = tagDBusInterface.data();
    connections << q->connect(ptr, &TagManagerDBusInterface::TagsServiceReady, q, &TagProxyHandle::tagServiceRegistered);
    connections << q->connect(ptr, &TagManagerDBusInterface::NewTagsAdded, q, &TagProxyHandle::newTagsAdded);
    connections << q->connect(ptr, &TagManagerDBusInterface::TagsDeleted, q, &TagProxyHandle::tagsDeleted);
    connections << q->connect(ptr, &TagManagerDBusInterface::TagsColorChanged, q, &TagProxyHandle::tagsColorChanged);
    connections << q->connect(ptr, &TagManagerDBusInterface::TagsNameChanged, q, &TagProxyHandle::tagsNameChanged);
    connections << q->connect(ptr, &TagManagerDBusInterface::FilesTagged, q, &TagProxyHandle::filesTagged);
    connections << q->connect(ptr, &TagManagerDBusInterface::FilesUntagged, q, &TagProxyHandle::filesUntagged);
}

void TagProxyHandlePrivate::disconnCurrentConnections()
{
    for (const auto &connection : connections)
        q->disconnect(connection);
    connections.clear();
}

QVariant TagProxyHandlePrivate::parseDBusVariant(const QDBusVariant &var)
{
    QVariant variant { var.variant() };
    QDBusArgument argument { variant.value<QDBusArgument>() };
    QDBusArgument::ElementType curType { argument.currentType() };
    QVariantMap varMap {};

    if (curType == QDBusArgument::ElementType::MapType) {
        argument >> varMap;
        variant.setValue(varMap);
    }

    return variant;
}

TagProxyHandle::TagProxyHandle(QObject *parent)
    : QObject(parent),
      d(new TagProxyHandlePrivate(this, parent))
{
}

TagProxyHandle *TagProxyHandle::instance()
{
    static TagProxyHandle ins;
    return &ins;
}

bool TagProxyHandle::isValid()
{
    return d->isDBusRuning();
}

QVariantMap TagProxyHandle::getAllTags()
{
    auto &&reply = d->tagDBusInterface->Query(int(QueryOpts::kTags));
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    const auto &data = d->parseDBusVariant(reply.value());
    return data.toMap();
}

QVariantMap TagProxyHandle::getTagsThroughFile(const QStringList &value)
{
    auto &&reply = d->tagDBusInterface->Query(int(QueryOpts::kTagsOfFile), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    const auto &data = d->parseDBusVariant(reply.value());
    return data.toMap();
}

QVariant TagProxyHandle::getSameTagsOfDiffFiles(const QStringList &value)
{
    auto &&reply = d->tagDBusInterface->Query(int(QueryOpts::kTagIntersectionOfFiles), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return d->parseDBusVariant(reply.value());
}

QVariantMap TagProxyHandle::getFilesThroughTag(const QStringList &value)
{
    auto &&reply = d->tagDBusInterface->Query(int(QueryOpts::kFilesOfTag), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    const auto &data = d->parseDBusVariant(reply.value());
    return data.toMap();
}

QVariantMap TagProxyHandle::getTagsColor(const QStringList &value)
{
    auto &&reply = d->tagDBusInterface->Query(int(QueryOpts::kColorOfTags), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    const auto &data = d->parseDBusVariant(reply.value());
    return data.toMap();
}

QVariantHash TagProxyHandle::getAllFileWithTags()
{
    auto &&reply = d->tagDBusInterface->Query(int(QueryOpts::kFilesWithTags));
    reply.waitForFinished();
    if (!reply.isValid()) {
        qWarning() << "getAllFileWithTags failed :" << reply.error();
        return {};
    }
    const auto &data = d->parseDBusVariant(reply.value());
    return data.toHash();
}

bool TagProxyHandle::addTags(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Insert(int(InsertOpts::kTags), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::addTagsForFiles(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Insert(int(InsertOpts::kTagOfFiles), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::changeTagsColor(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Update(int(UpdateOpts::kColors), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::changeTagNamesWithFiles(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Update(int(UpdateOpts::kTagsNameWithFiles), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::changeFilePaths(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Update(int(UpdateOpts::kFilesPaths), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::deleteTags(const QVariantMap &value)
{
    if (value.isEmpty())
        return false;

    auto &&reply = d->tagDBusInterface->Delete(int(DeleteOpts::kTags), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::deleteFiles(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Delete(int(DeleteOpts::kFiles), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::deleteFileTags(const QVariantMap &value)
{
    auto &&reply = d->tagDBusInterface->Delete(int(DeleteOpts::kTagOfFiles), value);
    reply.waitForFinished();
    if (!reply.isValid())
        return {};
    return reply.value();
}

bool TagProxyHandle::connectToService()
{
    qInfo() << "Start initilize dbus: `TagManagerDBusInterface`";
    d->tagDBusInterface.reset(new TagManagerDBusInterface(kTagService, kTagDBusPath,
                                                          QDBusConnection::sessionBus(), this));
    d->tagDBusInterface->setTimeout(3000);
    d->initConnection();
    return d->isDBusRuning();
}
