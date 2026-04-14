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
    // Do NOT update version here - incremental tasks should not change version number
    writeStatusJson(statusFilePath(), obj);
}

SERVICETEXTINDEX_END_NAMESPACE
