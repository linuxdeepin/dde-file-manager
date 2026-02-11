// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingmonitorfilter.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/fileutils.h>

#include <QDebug>
#include <QTextStream>

DPTAG_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

AnythingMonitorFilter &AnythingMonitorFilter::instance()
{
    static AnythingMonitorFilter ins;
    return ins;
}

bool AnythingMonitorFilter::whetherFilterCurrentPath(const QString &localPath)
{
    bool result { false };

    for (const QString &path : whiteList) {
        if (localPath == path || localPath.startsWith(path)
            || localPath.startsWith(FileUtils::bindPathTransform(path, true)))
            result = true;
    }

    for (const QString &path : blackList) {
        if (localPath == path || localPath.startsWith(path))
            result = false;
    }

    return result;
}

AnythingMonitorFilter::AnythingMonitorFilter(QObject *parent)
    : QObject(parent)
{
    readSettings();
}

AnythingMonitorFilter::~AnythingMonitorFilter()
{
}

void AnythingMonitorFilter::readSettings()
{
    QStringList invalidPath {};
    QStringList removedTilde {};

    whiteList = Application::genericSetting()->value("AnythingMonitorFilterPath", "WhiteList").toStringList();
    blackList = Application::genericSetting()->value("AnythingMonitorFilterPath", "BlackList").toStringList();

    readHomePathOfAllUsers();

    for (QString &path : blackList) {

        if (path.startsWith("~/")) {
            removedTilde.push_back(path);
            continue;
        }
    }

    for (const QString &path : removedTilde) {
        blackList.removeAll(path);
    }

    for (QString &path : removedTilde) {
        path.remove(0, 1);

        for (const auto &userNameAndHomePath : userNameAndHomePath)
            blackList.push_back(userNameAndHomePath.second + path);
    }

    for (const QString &path : blackList) {

        if (!DFMIO::DFile(path).exists()) {
            invalidPath.push_back(path);
        }
    }

    for (const QString &path : invalidPath) {
        blackList.removeAll(path);
    }

    invalidPath.clear();
    ///###: above this.
    ///###: remove '~' in configure. And replace '~' by the home path of every user.
    ///###: remove invalid path in the black-list.

    ///###: remove invalid path in white-list.
    ///###: make sure every in black-list is subdirectory of white-list.
    for (const QString &path : whiteList) {

        if (!DFMIO::DFile(path).exists()) {
            invalidPath.push_back(path);
        }
    }

    for (const QString &path : invalidPath) {
        whiteList.removeAll(path);
    }

    QList<QString>::const_iterator itrBeg {};
    QList<QString>::const_iterator itrEnd {};
    invalidPath.clear();

    ///###: here, start to jundge whether directories in black-list are subdirectories in white-list.
    for (const QString &dirInBlackList : blackList) {
        itrBeg = whiteList.cbegin();
        itrEnd = whiteList.cend();

        for (; itrBeg != itrEnd; ++itrBeg) {

            if (dirInBlackList.startsWith(*itrBeg)) {
                break;
            }
        }

        if (itrBeg == itrEnd) {
            invalidPath.push_back(dirInBlackList);
        }
    }

    for (const QString &path : invalidPath) {
        blackList.removeAll(path);
    }

    ///###:at last, maybe there are files in configure. So delete these.
    invalidPath.clear();

    reserveDir(&whiteList);
    reserveDir(&blackList);
}

void AnythingMonitorFilter::readHomePathOfAllUsers()
{
    if (DFMIO::DFile("/etc/passwd").exists()) {
        QFile file { "/etc/passwd" };

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            fmCritical("Can not open /etc/passwd!");
            return;
        }

        QTextStream in(&file);

        while (!in.atEnd()) {
            const QString &lineContent { in.readLine() };
            if (lineContent.isEmpty())
                continue;
            const QList<QString> &contents { lineContent.split(':') };
            if (contents.size() < 2)
                continue;

            const QString &pathRemoveEsc { restoreEscapedChar(contents[contents.size() - 2]) };
            userNameAndHomePath[contents[0]] = pathRemoveEsc;
        }

        file.close();
    }
}

QString AnythingMonitorFilter::restoreEscapedChar(const QString &value)
{
    static const std::map<QString, QString> kTableOfEscapeChar {
        { "\\007", "\a" },
        { "\\010", "\b" },
        { "\\014", "\f" },
        { "\\012", "\n" },
        { "\\015", "\r" },
        { "\\011", "\t" },
        { "\\013", "\v" },
        { "\\134", "\\" },
        { "\\047", "\'" },
        { "\\042", "\"" },
        { "\\040", " " }
    };

    QString tempValue { value };

    if (!tempValue.isEmpty() && !tempValue.isNull()) {
        std::map<QString, QString>::const_iterator tableBeg { kTableOfEscapeChar.cbegin() };
        std::map<QString, QString>::const_iterator tableEnd { kTableOfEscapeChar.cend() };

        for (; tableBeg != tableEnd; ++tableBeg) {
            int pos = tempValue.indexOf(tableBeg->first);

            if (pos != -1) {

                while (pos != -1) {
                    tempValue = tempValue.replace(tableBeg->first, tableBeg->second);
                    pos = tempValue.indexOf(tableBeg->first);
                }
            }
        }
    }

    return tempValue;
}

void AnythingMonitorFilter::reserveDir(QStringList *list)
{
    Q_ASSERT(list);

    QStringList pathInvalid;

    for (const QString &path : *list) {
        auto fileInfo { InfoFactory::create<FileInfo>(QUrl::fromLocalFile(path)) };

        if (fileInfo && !fileInfo->isAttributes(OptInfoType::kIsDir))
            pathInvalid.push_back(path);
    }

    for (const QString &path : pathInvalid)
        list->removeAll(path);
}

DPTAG_END_NAMESPACE
