// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexstatestore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {

/**
 * @brief 读取状态 JSON 文件并解析为 QJsonObject
 *
 * 状态文件采用 JSON 格式，包含以下 key（定义在 service_textindex_global.h 的 Defines 命名空间）：
 * - "state"          → "clean" 或 "dirty"（kStateKey → kStateClean / kStateDirty）
 * - "needsRebuild"   → true / false（kNeedsRebuildKey）
 * - "lastUpdateTime" → ISO 时间字符串（kLastUpdateTimeKey）
 * - "version"        → 整数版本号（kTextVersionKey / kOcrVersionKey，由 IndexProfile::versionKey() 提供）
 *
 * 文件不存在、无法打开、JSON 解析失败、根非 Object 时均返回空 QJsonObject（不抛异常）。
 * 调用方应能处理空对象（各 getter 有对应的默认返回值）。
 */
QJsonObject readStatusJson(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QJsonObject();
    }

    QJsonParseError parseError;
    const QByteArray data = file.readAll();
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        fmWarning() << "IndexStateStore::readStatusJson: failed to parse JSON from:" << file.fileName()
                    << "error:" << parseError.errorString();
        return QJsonObject();
    }

    if (!doc.isObject()) {
        fmWarning() << "IndexStateStore::readStatusJson: JSON root is not an object in:" << file.fileName();
        return QJsonObject();
    }

    return doc.object();
}

/**
 * @brief 将 QJsonObject 写入状态文件（原子写入）
 *
 * 使用 QSaveFile 实现原子写入：先写入临时文件，commit 时 rename 为目标文件。
 * 这样即使写入过程中进程崩溃，也不会产生半写入的损坏文件。
 * 如果目录不存在会先创建。
 */
bool writeStatusJson(const QString &filePath, const QJsonObject &obj)
{
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        fmWarning() << "IndexStateStore::writeStatusJson: failed to open file:" << file.fileName()
                    << "error:" << file.errorString();
        return false;
    }

    const QByteArray data = QJsonDocument(obj).toJson();
    if (file.write(data) != data.size()) {
        fmWarning() << "IndexStateStore::writeStatusJson: failed to fully write file:" << file.fileName();
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        fmWarning() << "IndexStateStore::writeStatusJson: failed to commit file:" << file.fileName()
                    << "error:" << file.errorString();
        return false;
    }

    return true;
}

}   // namespace

IndexStateStore::IndexStateStore(IndexProfile profile)
    : m_profile(std::move(profile))
{
}

QString IndexStateStore::statusFilePath() const
{
    return m_profile.statusFilePath();
}

IndexUtility::IndexState IndexStateStore::getIndexState() const
{
    const QJsonObject obj = readStatusJson(statusFilePath());
    if (obj.contains(Defines::kStateKey)) {
        const QString state = obj[Defines::kStateKey].toString();
        if (state == Defines::kStateClean) {
            return IndexUtility::IndexState::Clean;
        }
        if (state == Defines::kStateDirty) {
            return IndexUtility::IndexState::Dirty;
        }

        fmWarning() << "IndexStateStore::getIndexState: invalid state value for profile:" << m_profile.id()
                    << "value:" << state;
    }

    return IndexUtility::IndexState::Unknown;
}

void IndexStateStore::setIndexState(IndexUtility::IndexState state) const
{
    QString stateStr;
    switch (state) {
    case IndexUtility::IndexState::Clean:
        stateStr = Defines::kStateClean;
        break;
    case IndexUtility::IndexState::Dirty:
        stateStr = Defines::kStateDirty;
        break;
    default:
        fmWarning() << "IndexStateStore::setIndexState: unsupported state for profile:" << m_profile.id();
        return;
    }

    QJsonObject obj = readStatusJson(statusFilePath());
    obj[Defines::kStateKey] = stateStr;
    writeStatusJson(statusFilePath(), obj);
}

bool IndexStateStore::isCleanState() const
{
    return getIndexState() == IndexUtility::IndexState::Clean;
}

bool IndexStateStore::needsRebuild() const
{
    const QJsonObject obj = readStatusJson(statusFilePath());
    return obj.contains(Defines::kNeedsRebuildKey) ? obj[Defines::kNeedsRebuildKey].toBool() : false;
}

void IndexStateStore::setNeedsRebuild(bool need) const
{
    QJsonObject obj = readStatusJson(statusFilePath());
    obj[Defines::kNeedsRebuildKey] = need;
    writeStatusJson(statusFilePath(), obj);
}

bool IndexStateStore::isCreateInProgress() const
{
    const QJsonObject obj = readStatusJson(statusFilePath());
    return obj.contains(Defines::kCreateInProgressKey) ? obj[Defines::kCreateInProgressKey].toBool() : false;
}

void IndexStateStore::setCreateInProgress(bool inProgress) const
{
    QJsonObject obj = readStatusJson(statusFilePath());
    obj[Defines::kCreateInProgressKey] = inProgress;
    writeStatusJson(statusFilePath(), obj);
}

QString IndexStateStore::getLastUpdateTime() const
{
    const QJsonObject obj = readStatusJson(statusFilePath());
    if (!obj.contains(Defines::kLastUpdateTimeKey)) {
        return QString();
    }

    const QDateTime time = QDateTime::fromString(obj[Defines::kLastUpdateTimeKey].toString(), Qt::ISODate);
    return time.toString("yyyy-MM-dd hh:mm:ss");
}

int IndexStateStore::getIndexVersion() const
{
    const QJsonObject obj = readStatusJson(statusFilePath());
    return obj.contains(m_profile.versionKey()) ? obj[m_profile.versionKey()].toInt(-1) : -1;
}

bool IndexStateStore::isCompatibleVersion() const
{
    const int storedVersion = getIndexVersion();
    if (storedVersion == -1) {
        return false;
    }

    return storedVersion == m_profile.runtimeIndexVersion();
}

void IndexStateStore::removeIndexStatusFile() const
{
    QFile file(statusFilePath());
    if (file.exists()) {
        file.remove();
    }
}

void IndexStateStore::clearIndexDirectory() const
{
    QDir dir(m_profile.indexDirectory());
    if (dir.exists()) {
        const QStringList files = dir.entryList(QDir::Files);
        for (const QString &file : files) {
            dir.remove(file);
        }
    }

    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }
}

void IndexStateStore::saveIndexStatus(const QDateTime &lastUpdateTime) const
{
    saveIndexStatus(lastUpdateTime, m_profile.runtimeIndexVersion());
}

void IndexStateStore::saveIndexStatus(const QDateTime &lastUpdateTime, int version) const
{
    QJsonObject obj = readStatusJson(statusFilePath());
    obj[Defines::kLastUpdateTimeKey] = lastUpdateTime.toString(Qt::ISODate);
    obj[m_profile.versionKey()] = version;
    writeStatusJson(statusFilePath(), obj);
}

void IndexStateStore::saveLastUpdateTime(const QDateTime &lastUpdateTime) const
{
    QJsonObject obj = readStatusJson(statusFilePath());
    obj[Defines::kLastUpdateTimeKey] = lastUpdateTime.toString(Qt::ISODate);
    // 不更新 version：增量任务不应改变版本号。
    // 若在 recoveryPending 期间增量任务提升了 version，会导致后续恢复任务的版本判断出错。
    writeStatusJson(statusFilePath(), obj);
}

QStringList IndexStateStore::createFileListCache() const
{
    QMutexLocker locker(&m_createFileListCacheMutex);
    return m_createFileListCache;
}

void IndexStateStore::setCreateFileListCache(const QStringList &cache) const
{
    QMutexLocker locker(&m_createFileListCacheMutex);
    m_createFileListCache = cache;
}

int IndexStateStore::createCheckpoint() const
{
    return m_createCheckpoint.loadRelaxed();
}

void IndexStateStore::setCreateCheckpoint(int checkpoint) const
{
    m_createCheckpoint.storeRelaxed(checkpoint);
}

SERVICETEXTINDEX_END_NAMESPACE
