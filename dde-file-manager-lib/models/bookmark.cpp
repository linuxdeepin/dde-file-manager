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

#include "bookmark.h"
#include <QIcon>

BookMark::BookMark(const DUrl &url)
    : DFileInfo(url)
{
    m_name = url.fragment();
}

BookMark::BookMark(QDateTime time, const QString &name, const DUrl &sourceUrl)
    : DFileInfo(DUrl::fromBookMarkFile(sourceUrl.toString(), name))
{
    m_time = time;
    m_name = name;
}

BookMark::~BookMark()
{

}

QDateTime BookMark::getDateTime()
{
    return m_time;
}

DUrl BookMark::sourceUrl() const
{
    return DUrl(fileUrl().path());
}

void BookMark::setDateTime(QDateTime time)
{
    m_time = time;
}

void BookMark::setName(const QString &name)
{
    DUrl tmpUrl = fileUrl();
    tmpUrl.setFragment(name);
    setUrl(tmpUrl);
    m_name = name;
}

QString BookMark::getName() const
{
    return m_name;
}

QString BookMark::getDevcieId() const
{
    return m_devcieId;
}

void BookMark::setDevcieId(const QString &devcieId)
{
    m_devcieId = devcieId;
}

QString BookMark::getUuid() const
{
    return m_uuid;
}

void BookMark::setUuid(const QString &uuid)
{
    m_uuid = uuid;
}

QString BookMark::fileDisplayName() const
{
    return getName();
}

bool BookMark::canRedirectionFileUrl() const
{
    return true;
}

DUrl BookMark::redirectedFileUrl() const
{
    return sourceUrl();
}
