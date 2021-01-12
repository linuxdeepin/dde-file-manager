/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "trashjobcontroller.h"
#include "app/global.h"
#include "dialogs/cleartrashdialog.h"

TrashJobController::TrashJobController(QObject *parent) : QObject(parent)
{
    initConnect();
}

void TrashJobController::initConnect(){

}

void TrashJobController::asyncRequestTrashCount(){

}

void TrashJobController::setTrashEmptyFlag(bool flag){
    m_isTrashEmpty = flag;
}

void TrashJobController::trashJobExcute(const QStringList &files){
    qDebug() << files;
//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewTrashJob(
//                files,
//                false,
//                ConflictAdaptor::staticServerPath(),
//                m_conflictController->getObjectPath(),
//                ConflictAdaptor::staticInterfaceName()
//                );
////    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewTrashJob(
////                files,
////                false,
////                "",
////                "",
////                ""
////                );
//    reply.waitForFinished();
//    if (!reply.isError()){
//        QString service = reply.argumentAt(0).toString();
//        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
//        m_trashJobInterface = new TrashJobInterface(service, path, QDBusConnection::sessionBus(), this);
//        connectTrashSignal();
//        m_trashJobInterface->Execute();
//    }else{
//        qCritical() << reply.error().message() << files;
//    }
}

void TrashJobController::connectTrashSignal(){

}

void TrashJobController::disconnectTrashSignal(){

}

void TrashJobController::trashJobExcuteFinished(){

    qDebug() << "trash files deleted";
}


void TrashJobController::trashJobAbort(){

}

void TrashJobController::trashJobAbortFinished(){

}

void TrashJobController::onTrashingFile(QString /*file*/){

}

void TrashJobController::onDeletingFile(QString /*file*/){

}

void TrashJobController::onProcessAmount(qlonglong /*progress*/, ushort /*info*/){

}

void TrashJobController::updateTrashIconByCount(uint /*count*/){
//    if (count == 0){
//        if (!m_isTrashEmpty){
//            dbusController->asyncRequestTrashIcon();
//        }
//        m_isTrashEmpty = true;
//    }else{
//        if (m_isTrashEmpty){
//            dbusController->asyncRequestTrashIcon();
//        }
//        m_isTrashEmpty = false;
//    }
}


void TrashJobController::confirmDelete(){

}

void TrashJobController::handleTrashAction(int index){
    switch (index) {
    case 0:
        return;
        break;
    case 1:
        createEmptyTrashJob();
        break;
    default:
        break;
    }
}

void TrashJobController::createEmptyTrashJob(){
//    QDBusPendingReply<QString, QDBusObjectPath, QString> reply = dbusController->getFileOperationsInterface()->NewEmptyTrashJob(false, "", "", "");
//    reply.waitForFinished();
//    if (!reply.isError()){
//        QString service = reply.argumentAt(0).toString();
//        QString path = qdbus_cast<QDBusObjectPath>(reply.argumentAt(1)).path();
//        m_emptyTrashJobInterface = new EmptyTrashJobInterface(service, path, QDBusConnection::sessionBus(), this);
//        connect(m_emptyTrashJobInterface, SIGNAL(Done()), this, SLOT(emptyTrashJobExcuteFinished()));
//        m_emptyTrashJobInterface->Execute();
//    }else{
//        qCritical() << reply.error().message();
//    }
}

void TrashJobController::emptyTrashJobExcuteFinished(){

}

TrashJobController::~TrashJobController()
{

}

