// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagproxyhandle.h"
#include "private/tagproxyhandle_p.h"
#include "dbus/dbus_interface/tagdbus_interface.h"
#include "data/tagdbhandle.h"
#include "utils/tagmanager.h"

using namespace dfmplugin_tag;
static constexpr char kDesktopService[] { "org.deepin.filemanager.server" };
static constexpr char kTagDBusPath[] { "/org/deepin/filemanager/server/Tag" };

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
    // TODO(zhangs): refactor it! (bug-189717)
    static const QStringList kAppWhileList { "dde-file-manager", "dde-desktop" };
    if (!kAppWhileList.contains(qApp->applicationName()))
        return false;
    return tagDBusInterface && tagDBusInterface->isValid();
}

void TagProxyHandlePrivate::initConnection()
{
    dbusWatcher.reset(new QDBusServiceWatcher(kDesktopService, QDBusConnection::sessionBus()));
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceRegistered, q, [this] {
        connectToDBus();
    });
    q->connect(dbusWatcher.data(), &QDBusServiceWatcher::serviceUnregistered, q, [this] {
        connectToAPI();
    });

    if (isDBusRuning())
        connectToDBus();
    else
        connectToAPI();
}

void TagProxyHandlePrivate::connectToDBus()
{
    if (currentConnectionType == kDBusConnecting)
        return;
    disconnCurrentConnections();

    auto ptr = tagDBusInterface.data();
    connections << q->connect(ptr, &TagDBusInterface::NewTagsAdded, q, &TagProxyHandle::newTagsAdded);
    connections << q->connect(ptr, &TagDBusInterface::TagsDeleted, q, &TagProxyHandle::tagsDeleted);
    connections << q->connect(ptr, &TagDBusInterface::TagsColorChanged, q, &TagProxyHandle::tagsColorChanged);
    connections << q->connect(ptr, &TagDBusInterface::TagsNameChanged, q, &TagProxyHandle::tagsNameChanged);
    connections << q->connect(ptr, &TagDBusInterface::FilesTagged, q, &TagProxyHandle::filesTagged);
    connections << q->connect(ptr, &TagDBusInterface::FilesUntagged, q, &TagProxyHandle::filesUntagged);

    currentConnectionType = kDBusConnecting;
}

void TagProxyHandlePrivate::connectToAPI()
{
    if (currentConnectionType == kAPIConnecting)
        return;
    disconnCurrentConnections();

    auto ptr = TagDbHandle::instance();
    connections << q->connect(ptr, &TagDbHandle::newTagsAdded, q, &TagProxyHandle::newTagsAdded);
    connections << q->connect(ptr, &TagDbHandle::tagsDeleted, q, &TagProxyHandle::tagsDeleted);
    connections << q->connect(ptr, &TagDbHandle::tagsColorChanged, q, &TagProxyHandle::tagsColorChanged);
    connections << q->connect(ptr, &TagDbHandle::tagsNameChanged, q, &TagProxyHandle::tagsNameChanged);
    connections << q->connect(ptr, &TagDbHandle::filesWereTagged, q, &TagProxyHandle::filesTagged);
    connections << q->connect(ptr, &TagDbHandle::filesUntagged, q, &TagProxyHandle::filesUntagged);

    currentConnectionType = kAPIConnecting;
}

void TagProxyHandlePrivate::disconnCurrentConnections()
{
    for (const auto &connection : connections)
        q->disconnect(connection);
    connections.clear();
    currentConnectionType = kNoneConnection;
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

bool TagProxyHandle::canTagFile(const QString &filePath)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->CanTagFile(filePath);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagManager::instance()->canTagFile(QUrl(filePath));
    }
}

QVariantMap TagProxyHandle::getAllTags()
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Query(static_cast<std::size_t>(TagActionType::kGetAllTags));
        reply.waitForFinished();
        const auto &data = d->parseDBusVariant(reply.value());
        return data.toMap();
    } else {
        return TagDbHandle::instance()->getAllTags();
    }
}

QVariantMap TagProxyHandle::getTagsThroughFile(const QStringList &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Query(static_cast<std::size_t>(TagActionType::kGetTagsThroughFile), value);
        reply.waitForFinished();
        const auto &data = d->parseDBusVariant(reply.value());
        return data.toMap();
    } else {
        return TagDbHandle::instance()->getTagsByUrls(value);
    }
}

QVariant TagProxyHandle::getSameTagsOfDiffFiles(const QStringList &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Query(static_cast<std::size_t>(TagActionType::kGetSameTagsOfDiffFiles), value);
        reply.waitForFinished();
        return d->parseDBusVariant(reply.value());
    } else {
        return TagDbHandle::instance()->getSameTagsOfDiffUrls(value);
    }
}

QVariantMap TagProxyHandle::getFilesThroughTag(const QStringList &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Query(static_cast<std::size_t>(TagActionType::kGetFilesThroughTag), value);
        reply.waitForFinished();
        const auto &data = d->parseDBusVariant(reply.value());
        return data.toMap();
    } else {
        return TagDbHandle::instance()->getFilesByTag(value);
    }
}

QVariantMap TagProxyHandle::getTagsColor(const QStringList &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Query(static_cast<std::size_t>(TagActionType::kGetTagsColor), value);
        reply.waitForFinished();
        const auto &data = d->parseDBusVariant(reply.value());
        return data.toMap();
    } else {
        return TagDbHandle::instance()->getTagsColor(value);
    }
}

bool TagProxyHandle::addTags(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Insert(static_cast<std::size_t>(TagActionType::kAddTags), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->addTagProperty(value);
    }
}

bool TagProxyHandle::addTagsForFiles(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Insert(static_cast<std::size_t>(TagActionType::kMakeFilesTags), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->addTagsForFiles(value);
    }
}

bool TagProxyHandle::changeTagsColor(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Update(static_cast<std::size_t>(TagActionType::kChangeTagsColor), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->changeTagColors(value);
    }
}

bool TagProxyHandle::changeTagNamesWithFiles(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Update(static_cast<std::size_t>(TagActionType::kChangeTagsNameWithFiles), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->changeTagNamesWithFiles(value);
    }
}

bool TagProxyHandle::changeFilePaths(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Update(static_cast<std::size_t>(TagActionType::kChangeFilesPaths), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->changeFilePaths(value);
    }
}

bool TagProxyHandle::deleteTags(const QVariantMap &value)
{
    if (value.isEmpty())
        return false;

    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Delete(static_cast<std::size_t>(TagActionType::kDeleteTags), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->deleteTags(value.first().toStringList());
    }
}

bool TagProxyHandle::deleteFiles(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Delete(static_cast<std::size_t>(TagActionType::kDeleteFiles), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->deleteFiles(value.keys());
    }
}

bool TagProxyHandle::deleteFileTags(const QVariantMap &value)
{
    if (d->isDBusRuning()) {
        auto &&reply = d->tagDBusInterface->Delete(static_cast<std::size_t>(TagActionType::kRemoveTagsOfFiles), value);
        reply.waitForFinished();
        return reply.value();
    } else {
        return TagDbHandle::instance()->removeTagsOfFiles(value);
    }
}

bool TagProxyHandle::connectToService()
{
    qInfo() << "Start initilize dbus: `TagDBusInterface`";
    d->tagDBusInterface.reset(new TagDBusInterface(kDesktopService, kTagDBusPath, QDBusConnection::sessionBus(), this));
    d->initConnection();
    return d->isDBusRuning();
}
