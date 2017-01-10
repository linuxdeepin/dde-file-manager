#include "commandmanager.h"
#include "dbusadaptor/commandmanager_adaptor.h"
#include "../partman/command.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QtConcurrent>
#include <QFuture>
#include <QDebug>


QString CommandManager::ObjectPath = "/com/deepin/filemanager/daemon/CommandManager";

CommandManager::CommandManager(QObject *parent) : QObject(parent)
{
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_commandManagerAdaptor = new CommandManagerAdaptor(this);
}
using namespace PartMan;

bool CommandManager::process(const QString &cmd, const QStringList &args,  QString &output,  QString &error)
{
    typedef bool (*Exec) (const QString &, const QStringList & , QString &,  QString &);
    Exec f = &PartMan::SpawnCmd;
    QFuture<bool> future = QtConcurrent::run(f, cmd, args, output, error);
    future.waitForFinished();
    bool ret = future.result();
    return ret;
}

bool CommandManager::startDetached(const QString &cmd, const QStringList &args)
{
    bool ret = QProcess::startDetached(cmd, args);
    qDebug() << ret << cmd << args;
    return ret;
}
