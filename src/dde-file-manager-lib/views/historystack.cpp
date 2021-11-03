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

#include "historystack.h"

#include "dfileservices.h"
#include "plugins/pluginmanager.h"
#include "drootfilemanager.h"

#include <QDebug>
#include <QProcess>

HistoryStack::HistoryStack(int threshold)
{
    m_threshold = threshold;
    m_index = -1;
}

void HistoryStack::append(DUrl url)
{
    if ((m_index < m_list.count()) && (m_index >= 0)) {
        if(m_list.at(m_index) == url)
            return;
    }

    if (m_index < m_threshold) {
        ++m_index;

        if (m_index != m_list.size()) {
            m_list = m_list.mid(0, m_index);
        }

        m_list.append(url);
    } else {
        m_list.takeFirst();
        m_list.append(url);
    }
}

DUrl HistoryStack::back()
{
    const DUrl &currentUrl = m_list.value(m_index);
    DUrl url;

    if (m_index <= 0)
        return url;

    while (--m_index >= 0) {
        if (m_index >= m_list.count())
            continue;

        url = m_list.at(m_index);

        if(url.isComputerFile())
            break;

        //TODO [XIAO] 此处可以用插件的方式写
        //如果是我的手机界面返回,为了我的手机界面前进，回退功能
        if (url.isPluginFile())
            break;

        if (url.isUserShareFile())
            break;

        if (PluginManager::instance()->getViewInterfacesMap().keys().contains(url.scheme()))
            break;
        //判断网络文件是否可以到达
        if (DFileService::instance()->checkGvfsMountfileBusy(url,false)) {
            if (currentUrl == url) {
                removeAt(m_index);
                url = m_list.at(m_index);
            }
            else {
                break;
            }
        }
        else {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

            if (!fileInfo || !fileInfo->exists() || currentUrl == url) {
                removeAt(m_index);
                url = m_list.at(m_index);
            } else {
                break;
            }
        }
    }

    return url;
}

DUrl HistoryStack::forward()
{
    const DUrl &currentUrl = m_list.value(m_index);
    DUrl url;

    if (m_index >= m_list.count() - 1)
        return url;

    while (++m_index < m_list.count()) {
        url = m_list.at(m_index);

        if(url.isComputerFile())
            break;

        //TODO [XIAO] 此处可以用插件的方式写
        //如果是我的手机界面返回,为了我的手机界面前进，回退功能
        if (url.isPluginFile())
            break;

        if (url.isUserShareFile())
            break;

        if (PluginManager::instance()->getViewInterfacesMap().keys().contains(url.scheme()))
            break;
        //判断网络文件是否可以到达
        if (DFileService::instance()->checkGvfsMountfileBusy(url,false)) {
            if (currentUrl == url) {
                removeAt(m_index);
                --m_index;
                url = m_list.at(m_index);
            }
            else {
                break;
            }
        }
        else {
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);

            if (!fileInfo || !fileInfo->exists() || currentUrl == url) {
                removeAt(m_index);
                --m_index;
                url = m_list.at(m_index);
            } else {
                break;
            }
        }
    }

    return url;
}

void HistoryStack::setThreshold(int threshold)
{
    m_threshold = threshold;
}

bool HistoryStack::isFirst()
{
    if(m_index < 0){
        m_index = 0;
    }
    return m_index == 0;
}

bool HistoryStack::isLast()
{
    if(m_index > m_list.size() - 1){
        m_index = m_list.size() - 1;
    }
    return m_index == m_list.size() - 1;
}

int HistoryStack::size()
{
    return m_list.size();
}

void HistoryStack::removeAt(int i)
{
    m_list.removeAt(i);
}

int HistoryStack::currentIndex()
{
    return m_index;
}

bool HistoryStack::backIsExist()
{
    if (m_index <= 0)
        return false;

    DUrl backUrl = m_list.at(m_index - 1);

    if (!needCheckExist(backUrl))
        return true;

    //为了避免获取目录的info对象，采用了test命令，只测试计算机中是否包含该目录
    //避免获取info对象是因为若目录为网络目录，且该网络已不可达，会导致获取info等待时间不可控
    return checkPathIsExist(backUrl);
}

bool HistoryStack::forwardIsExist()
{
    if (m_index >= m_list.size() - 1)
        return false;

    DUrl forwardUrl = m_list.at(m_index + 1);

    if (!needCheckExist(forwardUrl))
        return true;

    //为了避免获取目录的info对象，采用了test命令，只测试计算机中是否包含该目录
    //避免获取info对象是因为若目录为网络目录，且该网络已不可达，会导致获取info等待时间不可控
    return checkPathIsExist(forwardUrl);
}

bool HistoryStack::needCheckExist(const DUrl &url)
{
    if (url.isComputerFile() || url.isUserShareFile())
        return false;

    if (PluginManager::instance()->getViewInterfacesMap().keys().contains(url.scheme()))
        return false;

    return true;
}

bool HistoryStack::checkPathIsExist(const DUrl &url)
{
    //判断是否是协议设备的挂载路径
    static QRegularExpression regExp(GVFS_MATCH_EX,
                                     QRegularExpression::DotMatchesEverythingOption
                                     | QRegularExpression::DontCaptureOption
                                     | QRegularExpression::OptimizeOnFirstUsageOption);

    QString urlPath = url.path();
    if (regExp.match(urlPath, 0, QRegularExpression::NormalMatch, QRegularExpression::DontCheckSubjectStringMatchOption).hasMatch()) {
        //协议设备需要考虑断网问题，通过查询挂载点是否存在来判断是否存在
        DUrl rootUrl;
        QString rootFileName;
        int qi = 0;
        QStringList urlPathList = urlPath.split(QRegularExpression(GVFS_MATCH));
        QString urlStr;
        QString urlLast;
        if (urlPathList.size() >= 2) {
            urlLast = urlPathList[1];
            urlStr = urlPath.left(urlPath.indexOf(urlLast));
        }

        qi = urlLast.indexOf("/");
        QString path;
        if (0 >= qi) {
            path = urlPath;
            QStringList rootStrList = path.split(QRegularExpression(GVFS_MATCH));
            if (rootStrList.size() >= 2) {
                rootFileName = rootStrList.at(1);
                rootFileName = rootFileName.replace(QString(".") + QString(SUFFIX_GVFSMP), "");
            }
        } else {
            rootFileName = urlLast.left(qi);
            path = urlStr + urlLast.left(qi);
        }
        if (path.isNull() || path.isEmpty() ||
                !(rootFileName.startsWith(SMB_SCHEME) || rootFileName.startsWith(FTP_SCHEME)
                  || rootFileName.startsWith(SFTP_SCHEME) || rootFileName.startsWith(MTP_SCHEME)
                  || rootFileName.startsWith(GPHOTO2_SCHEME) || rootFileName.startsWith(AFC_SCHEME))) {
            return false;
        }
        rootUrl.setScheme(DFMROOT_SCHEME);
        rootUrl.setPath("/" + QUrl::toPercentEncoding(path) + "." SUFFIX_GVFSMP);

        return DRootFileManager::instance()->isRootFileContain(rootUrl);
    } else {
        // 对我的手机目录进行单独处理，直接返回true，否则涉及我的手机相关目录前进后退会失效 add by CL
        if (url.scheme() == PLUGIN_SCHEME) {
            return true;
        }
        //非协议设备挂载不用考虑断网问题，可以直接取fileinfo来判断是否存在
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, url);
        if (fileInfo)
            return fileInfo->exists();
    }

    return false;
}

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug beg, const HistoryStack &stack)
{
    beg << "history list:" << stack.m_list << "current pos:" << stack.m_index;

    return beg;
}
QT_END_NAMESPACE
