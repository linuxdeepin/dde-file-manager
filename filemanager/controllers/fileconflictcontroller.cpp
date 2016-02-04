#include "fileconflictcontroller.h"
#include <QDBusConnection>
#include <QDebug>
#include "../app/global.h"

int FileConflictController::count = 0;

FileConflictController::FileConflictController(QObject *parent):QObject(parent)
{
    FileConflictController::count += 1;
    m_objectPath = QString("%1%2").arg(ConflictAdaptor::staticInterfacePath(), QString::number(FileConflictController::count));
    registerDBusService();
}

void FileConflictController::registerDBusService(){
    m_conflictAdaptor = new ConflictAdaptor(this);
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.registerService(ConflictAdaptor::staticServerPath());
    bool flag = conn.registerObject(m_objectPath, this);
    qDebug() << "register conflict DBusService"<<flag;
}

ConflictInfo FileConflictController::AskSkip(const QString &primaryText,
                                     const QString &secondaryText,
                                     const QString &detailText,
                                     bool retry,
                                     bool multi){
    Q_UNUSED(primaryText)
    Q_UNUSED(secondaryText)
    Q_UNUSED(detailText)
    Q_UNUSED(retry)
    Q_UNUSED(multi)
    qDebug() << "AskSkip";
    ConflictInfo obj;
    obj.code = 1;
    obj.applyToAll = false;
    obj.userData = "";
    m_conflictAdaptor->response(obj);
    return obj;
}


ConflictInfo FileConflictController::ConflictDialog(){
    qDebug() << "=====ConflictDialog======";
    emit signalManager->conflictDialogShowed(m_jobDetail);
    qDebug() << m_jobDetail;
    ConflictInfo obj;
    obj.code = 32;
    obj.applyToAll = false;
    obj.userData = "";
    return obj;
}

QString FileConflictController::getObjectPath(){
    return m_objectPath;
}

ConflictAdaptor* FileConflictController::getConflictAdaptor(){
    return m_conflictAdaptor;
}

ConflictInfo FileConflictController::AskDelete(const QString &primaryText, const QString &secondaryText, const QString &detailText, bool retry, bool multi)
{
    Q_UNUSED(primaryText)
    Q_UNUSED(secondaryText)
    Q_UNUSED(detailText)
    Q_UNUSED(retry)
    Q_UNUSED(multi)
    qDebug() << "AskDelete";
    ConflictInfo obj;
    obj.code = 8;
    obj.applyToAll = true;
    obj.userData = "";
    m_conflictAdaptor->response(obj);
    return obj;
}

bool FileConflictController::AskDeleteConfirmation(const QString &primaryText, const QString &secondaryText, const QString &detailText)
{
    Q_UNUSED(primaryText)
    Q_UNUSED(secondaryText)
    Q_UNUSED(detailText)
    qDebug() << "AskDeleteConfirmation";
    return false;
}

ConflictInfo FileConflictController::AskRetry(const QString &primaryText, const QString &secondaryText, const QString &detailText)
{
    Q_UNUSED(primaryText)
    Q_UNUSED(secondaryText)
    Q_UNUSED(detailText)
    qDebug() << "AskRetry";
    ConflictInfo obj;
    obj.code = 4;
    obj.applyToAll = false;
    obj.userData = "";
    m_conflictAdaptor->response(obj);
    return obj;
}

void FileConflictController::unRegisterDBusService(){
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.unregisterObject(m_objectPath);
}

void FileConflictController::setJobDetail(const QMap<QString, QString> &detail){
    m_jobDetail = detail;
}

FileConflictController::~FileConflictController()
{
}
