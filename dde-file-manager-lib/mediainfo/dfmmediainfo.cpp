/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#include "dfmmediainfo.h"
#include "MediaInfo/MediaInfo.h"
#include <QApplication>
#include <QTimer>

#define MediaInfo_State_Finished 10000
using namespace MediaInfoLib;

DFM_BEGIN_NAMESPACE
class DFMMediaInfoPrivate : public QSharedData {
public:
    DFMMediaInfoPrivate(DFMMediaInfo *qq, const QString &file) : q_ptr(qq){
        m_mediaInfo = new MediaInfo;
        m_mediaInfo->Option(__T("Thread"), __T("1")); // open file in thread..
        m_mediaInfo->Option(__T("Inform"), __T("Text"));
        m_mediaInfo->Open(file.toStdWString());
        m_timer = new QTimer(qq);
        m_timer->setInterval(200);
        m_timer->start();
        QObject::connect(m_timer, &QTimer::timeout, qq, [this](){
            if (m_mediaInfo) {
                if (m_mediaInfo->State_Get() == MediaInfo_State_Finished) {
                    emit q_ptr->Finished();
                    m_timer->stop();
                    return ;
                }
//                if (m_mediaInfo->Count_Get(Stream_Image)>0)
//                    emit q_ptr->typeFinished("image");
//                if (m_mediaInfo->Count_Get(Stream_Video)>0)
//                    emit q_ptr->typeFinished("video");
//                if (m_mediaInfo->Count_Get(Stream_Audio)>0)
//                    emit q_ptr->typeFinished("audio");
            }
        });
    }

    ~DFMMediaInfoPrivate(){
        if (m_timer)
            m_timer->stop();
        if (m_mediaInfo)
            delete m_mediaInfo;
    }

    QString Inform(){
        //m_mediaInfo->Option(__T("Inform"), __T("Text"));
        qDebug() << "state:" << m_mediaInfo->State_Get(); // 10000 is ok?
        QString info = QString::fromStdWString(m_mediaInfo->Inform());
        return info;
    }

    QString Value(const QString &key, stream_t type){
        QString info = QString::fromStdWString(m_mediaInfo->Get(type, 0, key.toStdWString()));
        return info;
    }

private:
    MediaInfo     *m_mediaInfo {nullptr};
    QTimer        *m_timer {nullptr};
    DFMMediaInfo *q_ptr{ nullptr };
    Q_DECLARE_PUBLIC(DFMMediaInfo)
};

DFMMediaInfo::DFMMediaInfo(const QString &filename, QObject *parent)
    : QObject (parent)
    , d_private(new DFMMediaInfoPrivate(this, filename))
{

}

DFMMediaInfo::~DFMMediaInfo()
{

}

QString DFMMediaInfo::generalInformation(const QString &filename)
{
    DFMMediaInfoPrivate info(nullptr ,filename);
    return info.Inform();
}

QString DFMMediaInfo::Value(const QString &key, MeidiaType meidiaType/* = General*/)
{
    Q_D(DFMMediaInfo);
    return d->Value(key, static_cast<stream_t>(meidiaType));
}

DFM_END_NAMESPACE
