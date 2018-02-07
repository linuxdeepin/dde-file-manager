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

#include "copyjobworker.h"
#include "app/global.h"

CopyjobWorker::CopyjobWorker(QStringList files, QString destination, QObject *parent) :
    QObject(parent),
    m_files(files),
    m_destination(destination)
{
    m_progressTimer = new QTimer;
    m_progressTimer->setInterval(1000);
    m_time = new QTime;
    initConnect();
}

void CopyjobWorker::initConnect(){
    connect(this, SIGNAL(startJob()), this, SLOT(start()));
    connect(this, SIGNAL(finished()), this, SLOT(handleFinished()));
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
}

QStringList CopyjobWorker::getFiles(){
    return m_files;
}

QString CopyjobWorker::getDestination(){
    return m_destination;
}

QString CopyjobWorker::getJobPath(){
    return m_copyjobPath;
}

const QMap<QString, QString>& CopyjobWorker::getJobDetail(){
    return m_jobDetail;
}

FileConflictController* CopyjobWorker::getFileConflictController(){
    return m_conflictController;
}

void CopyjobWorker::start(){
    copyFiles(m_files, m_destination);
}

void CopyjobWorker::copyFiles(QStringList files, QString destination){
    Q_UNUSED(destination)
    if (files.length() == 0)
        return;
}


void CopyjobWorker::connectCopyJobSignal(){

}


void CopyjobWorker::disconnectCopyJobSignal(){

}


void CopyjobWorker::copyJobExcuteFinished(QString file){
    disconnectCopyJobSignal();
    m_copyJobInterface = NULL;
    m_progressTimer->stop();
    emit finished();
    qDebug() << "copy job finished" << file;
}

void CopyjobWorker::copyJobAbort(){
    if (m_copyJobInterface){
        m_progressTimer->stop();
    }
}

void CopyjobWorker::copyJobAbortFinished(){
    qDebug() << "copy job aborted";
    copyJobExcuteFinished("");
}

void CopyjobWorker::onCopyingFile(QString /*file*/){

}

void CopyjobWorker::setTotalAmount(qlonglong amount, ushort type){
    qDebug() << "========="<< amount << type;
    if (type == 0){
        m_totalAmout = amount;
    }
}

void CopyjobWorker::onCopyingProcessAmount(qlonglong progress, ushort type){
//    qDebug() << "onCopyingProcessAmount" << this << m_totalAmout << progress << type;
    if (type == 0){
        m_currentProgress = progress;
    }
}

void CopyjobWorker::onProcessedPercent(qlonglong percent){
    m_processedPercent = percent;
    m_jobDataDetail.insert("progress", QString::number(percent));
}

void CopyjobWorker::handleTimeout(){
    float speed;
    int remainTime;
    QString speedString;
    if (m_currentProgress - m_lastProgress > 1024 *1024){
        speed = (m_currentProgress - m_lastProgress) / (1024 * 1024);
        speedString = QString("%1 M/s").arg(QString::number(speed));
    }else{
        speed = (m_currentProgress - m_lastProgress) / 1024;
        speedString = QString("%1 Kb/s").arg(QString::number(speed));
    }
    if (m_currentProgress - m_lastProgress > 0){
        remainTime = (m_totalAmout - m_currentProgress) / (m_currentProgress - m_lastProgress);
    }
    m_lastProgress = m_currentProgress;

    m_jobDataDetail.insert("speed", speedString);
    m_jobDataDetail.insert("remainTime", QString("%1 s").arg(QString::number(remainTime)));
}

void CopyjobWorker::handleFinished(){

}

void CopyjobWorker::handleTaskAborted(const QMap<QString, QString> &jobDetail){
    if (jobDetail == m_jobDetail){
        copyJobAbort();
    }
}

void CopyjobWorker::stopTimer(){
    m_progressTimer->stop();
}

void CopyjobWorker::restartTimer(){
    m_progressTimer->start();
}
