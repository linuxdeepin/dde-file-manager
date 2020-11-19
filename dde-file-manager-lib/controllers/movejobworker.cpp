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

#include "movejobworker.h"
#include "app/define.h"
#include "utils/utils.h"

#include <QDebug>

MovejobWorker::MovejobWorker(QStringList files, QString destination, QObject *parent) :
    QObject(parent),
    m_files(files),
    m_destination(destination)
{
    m_progressTimer = new QTimer;
    m_progressTimer->setInterval(2000);
    m_time = new QTime;
    initConnect();
}

void MovejobWorker::initConnect(){
    connect(this, SIGNAL(startJob()), this, SLOT(start()));
    connect(this, SIGNAL(finished()), this, SLOT(handleFinished()));
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
}

QStringList MovejobWorker::getFiles(){
    return m_files;
}

QString MovejobWorker::getDestination(){
    return m_destination;
}

QString MovejobWorker::getJobPath(){
    return m_movejobPath;
}

const QMap<QString, QString>& MovejobWorker::getJobDetail(){
    return m_jobDetail;
}

FileConflictController* MovejobWorker::getFileConflictController(){
    return m_conflictController;
}


void MovejobWorker::start(){
    moveFiles(m_files, m_destination);
}

void MovejobWorker::moveFiles(QStringList files, QString destination){
    qDebug() << files << destination;
}

void MovejobWorker::connectMoveJobSignal(){

}


void MovejobWorker::disconnectMoveJobSignal(){

}


void MovejobWorker::moveJobExcuteFinished(const QString& message){

    qDebug() << "move job finished" << message;
}

void MovejobWorker::moveJobAbort(){

}

void MovejobWorker::moveJobAbortFinished(){
    moveJobExcuteFinished("move job aborted");
}

void MovejobWorker::onMovingFile(QString file){
    m_jobDataDetail.insert("file", QFileInfo(decodeUrl(file)).fileName());
    qDebug() << "onMovingFile" << file;
    if (m_jobDetail.contains("jobPath")){

    }
}

void MovejobWorker::setTotalAmount(qlonglong amount, ushort type){
    qDebug() << "========="<< amount << type;
    if (type == 0){
        m_totalAmout = amount;
    }
}

void MovejobWorker::onMovingProcessAmount(qlonglong progress, ushort info){
    if (info == 0){
        m_currentProgress = progress;
    }
    qDebug() << "onMovingProcessAmount" << progress << info;
}

void MovejobWorker::handleTimeout(){
    float speed = (m_currentProgress - m_lastProgress) / (1024 * 1024);
//    qDebug() << speed;
    m_lastProgress = m_currentProgress;
    int remainTime = (m_totalAmout - m_currentProgress) / speed;
//    qDebug() << remainTime;
    m_jobDataDetail.insert("speed", QString::number(speed));
    m_jobDataDetail.insert("remainTime", QString::number(remainTime));
}

void MovejobWorker::handleFinished(){
    if (m_jobDetail.contains("jobPath")){

    }
}

void MovejobWorker::handleTaskAborted(const QMap<QString, QString> &jobDetail){
    if (jobDetail == m_jobDetail){
        moveJobAbort();
    }
}
