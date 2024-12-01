// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textindexdbus.h"
#include "private/textindexdbus_p.h"

static constexpr char kTextIndexObjPath[] { "/org/deepin/Filemanager/TextIndex" };

SERVICETEXTINDEX_BEGIN_NAMESPACE
DFM_LOG_REISGER_CATEGORY(SERVICETEXTINDEX_NAMESPACE)
SERVICETEXTINDEX_END_NAMESPACE

SERVICETEXTINDEX_USE_NAMESPACE
using namespace Lucene;

void TextIndexDBusPrivate::initConnect()
{
    QObject::connect(taskManager, &TaskManager::taskFinished,
                     q, [this](const QString &type, const QString &path, bool success) {
        emit q->TaskFinished(type, path, success);
    });

    QObject::connect(taskManager, &TaskManager::taskProgressChanged,
                     q, [this](const QString &type, const QString &path, qint64 count) {
        emit q->TaskProgressChanged(type, path, count);
    });
}

TextIndexDBus::TextIndexDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext(), d(new TextIndexDBusPrivate(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SessionBus, QString(name)).registerObject(kTextIndexObjPath, this, opts);
}

TextIndexDBus::~TextIndexDBus() { }

bool TextIndexDBus::CreateIndexTask(const QString &path)
{
    return d->taskManager->startTask(IndexTask::Type::Create, path);
}

bool TextIndexDBus::UpdateIndexTask(const QString &path)
{
    return d->taskManager->startTask(IndexTask::Type::Update, path);
}

bool TextIndexDBus::RemoveIndexTask(const QStringList &paths)
{
    QString pathString = paths.join("|");
    return d->taskManager->startTask(IndexTask::Type::Remove, pathString);
}

bool TextIndexDBus::StopCurrentTask()
{
    if (!d->taskManager->hasRunningTask())
        return false;

    d->taskManager->stopCurrentTask();
    return true;
}

bool TextIndexDBus::HasRunningTask()
{
    return d->taskManager->hasRunningTask();
}

bool TextIndexDBus::IndexDatabaseExists()
{
    return IndexReader::indexExists(FSDirectory::open(indexStorePath().toStdWString()));
}

QString TextIndexDBus::GetLastUpdateTime()
{
    return d->taskManager->getLastUpdateTime();
}
