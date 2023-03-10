// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dockutils.h"

#include <QStringListIterator>
#include <QRegularExpression>

QString size_format::formatDiskSize(const quint64 num)
{
    QStringList list { " B", " KB", " MB", " GB", " TB" };
    qreal fileSize(num);

    QStringListIterator i(list);
    QString unit = i.next();

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }

    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', 1)), unit);
}

QString size_format::sizeString(const QString &str)
{
    int beginPos = str.indexOf('.');

    if (beginPos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > beginPos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

bool smb_utils::parseSmbInfo(const QString &smbPath, QString &host, QString &share)
{
    static const QRegularExpression regx(R"(([:,]port=(?<port>\d*))?[,:]server=(?<host>[^/:,]+)(,share=(?<share>[^/:,]+))?)");
    auto match = regx.match(smbPath);
    if (!match.hasMatch())
        return false;

    host = match.captured("host");
    share = match.captured("share");
    return true;
}
