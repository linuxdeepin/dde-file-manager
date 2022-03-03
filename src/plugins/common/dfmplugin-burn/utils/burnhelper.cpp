/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "burnhelper.h"

#include <DDialog>
#include <QObject>
#include <QCoreApplication>
#include <QPushButton>
#include <QStandardPaths>
#include <QRegularExpression>

DPBURN_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int BurnHelper::showOpticalBlankConfirmationDialog()
{
    QString EraseDisk = QObject::tr("Are you sure you want to erase all data on the disc?");
    QStringList buttonTexts;
    buttonTexts.append(QObject::tr("Cancel", "button"));
    buttonTexts.append(QObject::tr("Erase", "button"));

    DDialog d;

    if (!d.parentWidget()) {
        d.setWindowFlags(d.windowFlags() | Qt::WindowStaysOnTopHint);
    }

    d.setTitle(EraseDisk);
    d.setMessage(QObject::tr("This action cannot be undone"));
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(64, 64));
    d.addButton(buttonTexts[0], true, DDialog::ButtonNormal);
    d.addButton(buttonTexts[1], false, DDialog::ButtonWarning);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.moveToCenter();
    int code = d.exec();

    return code;
}

QUrl BurnHelper::localStagingFile(QString dev)
{
    // TODO(zhangs): replace it
#define DISCBURN_STAGING "discburn"
    return QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation)   // ~/.cache
                               + "/" + qApp->organizationName() + "/" DISCBURN_STAGING "/"   // ~/.cache/deepin/discburn/
                               + dev.replace('/', '_'));
}

QString BurnHelper::parseXorrisoErrorMessage(const QStringList &msg)
{
    QRegularExpression ovrex("While grafting '(.*)'");
    for (auto &msgs : msg) {
        auto ovrxm = ovrex.match(msgs);
        if (msgs.contains("file object exists and may not be overwritten") && ovrxm.hasMatch()) {
            return QObject::tr("%1 is a duplicate file.").arg(ovrxm.captured(1));
        }
        if (msgs.contains(QRegularExpression("Image size [0-9s]* exceeds free space on media [0-9s]*"))) {
            return QObject::tr("Insufficient disc space.");
        }
        if (msgs.contains("Lost connection to drive")) {
            return QObject::tr("Lost connection to drive.");
        }
        if (msgs.contains("servo failure")) {
            return QObject::tr("The CD/DVD drive is not ready. Try another disc.");
        }
        if (msgs.contains("Device or resource busy")) {
            return QObject::tr("The CD/DVD drive is busy. Exit the program using the drive, and insert the drive again.");
        }
        if (msgs.contains("-volid: Text too long")) {
            //something is wrong if the following return statement is reached.
            return QString("invalid volume name");
        }
    }
    return QObject::tr("Unknown error");
}
