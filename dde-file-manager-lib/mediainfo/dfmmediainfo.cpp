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
using namespace MediaInfoLib;

DFM_BEGIN_NAMESPACE

class DFMMediaInfoPrivate : public QSharedData{
public:
    DFMMediaInfoPrivate(DFMMediaInfo *qq, const QString &file) : q_ptr(qq){
        m_mediaInfo.Open(file.toStdWString());
    }

    ~DFMMediaInfoPrivate(){
         m_mediaInfo.Close();
    }

    QString Option(const QString &opt= "Complete", QString value = "1"){
        QString res = QString::fromStdWString(m_mediaInfo.Option(opt.toStdWString(), value.toStdWString()));
        return res;
    }

    QString Inform(){
        return QString::fromStdWString(m_mediaInfo.Inform());
    }

    QString Value(const QString &key, stream_t type){
        return QString::fromStdWString(m_mediaInfo.Get(type, 0, key.toStdWString()));
    }

private:
    MediaInfo m_mediaInfo;

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
