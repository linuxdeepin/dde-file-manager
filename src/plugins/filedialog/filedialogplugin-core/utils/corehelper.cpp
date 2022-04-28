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
#include "corehelper.h"

#include "dfm-base/dfm_event_defines.h"

#include <DDialog>
#include <DPlatformWindowHandle>

#include <QObject>
#include <QLabel>

DIALOGCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#if DTK_VERSION > DTK_VERSION_CHECK(2, 0, 5, 0)
static bool pwPluginVersionGreaterThen(const QString &v)
{
    const QStringList &versionList = DPlatformWindowHandle::pluginVersion().split(".");
    const QStringList &vList = v.split(".");

    for (int i = 0; i < versionList.count(); ++i) {
        if (v.count() <= i)
            return true;

        if (versionList[i].toInt() > vList[i].toInt())
            return true;
    }

    return false;
}
#endif

void CoreHelper::installDFMEventFilterForReject()
{
    // reject follow events
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kOpenNewWindow, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kOpenNewTab, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kOpenAsAdmin, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kOpenFilesByApp, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kCreateSymlink, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kOpenInTerminal, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
    dpfInstance.eventDispatcher().installEventFilter(GlobalEventType::kHideFiles, [](EventDispatcher::Listener, const QVariantList &) {
        return true;
    });
}

/*!
 * \brief Files with filenames starting with a dot are considered as hidden files and need to be checked
 * \return true if don't save as hidden file
 */
bool CoreHelper::askHiddenFile(QWidget *parent)
{
    DDialog dialog(parent);

    dialog.setIcon(QIcon::fromTheme("dialog-warning"));
    dialog.setTitle(QObject::tr("This file will be hidden if the file name starts with a dot (.). Do you want to hide it?"));
    dialog.addButton(QObject::tr("Cancel", "button"), true);
    dialog.addButton(QObject::tr("Confirm", "button"), false, DDialog::ButtonWarning);

    if (dialog.exec() != DDialog::Accepted)
        return true;

    return false;
}

/*!
 * \brief FileDialogPrivate::askReplaceFile
 * \param fileName
 * \return true if don't replace file
 */
bool CoreHelper::askReplaceFile(QString fileName, QWidget *parent)
{
    DDialog dialog(parent);

    // NOTE(zccrs): dxcb bug
    if ((!WindowUtils::isWayLand() && !DPlatformWindowHandle::isEnabledDXcb(parent))
#if DTK_VERSION > DTK_VERSION_CHECK(2, 0, 5, 0)
        || pwPluginVersionGreaterThen("1.1.8.3")
#endif
    ) {
        dialog.setWindowModality(Qt::WindowModal);
    }

    dialog.setIcon(QIcon::fromTheme("dialog-warning"));

    QLabel *titleLabel = dialog.findChild<QLabel *>("TitleLabel");
    if (titleLabel)
        fileName = titleLabel->fontMetrics().elidedText(fileName, Qt::ElideMiddle, 380);

    QString title = QObject::tr("%1 already exists, do you want to replace it?").arg(fileName);
    dialog.setTitle(title);
    dialog.addButton(QObject::tr("Cancel", "button"), true);
    dialog.addButton(QObject::tr("Replace", "button"), false, DDialog::ButtonWarning);

    if (dialog.exec() != DDialog::Accepted)
        return true;

    return false;
}

/*!
 * \brief Strip the filters by removing the details, e.g. (*.*).
 */
QStringList CoreHelper::stripFilters(const QStringList &filters)
{
    QStringList strippedFilters;
    QRegExp r(QString::fromLatin1("^(.*)\\(([^()]*)\\)$"));
    const int numFilters = filters.count();
    strippedFilters.reserve(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        QString filterName = filters[i];
        int index = r.indexIn(filterName);
        if (index >= 0) {
            filterName = r.cap(1);
        }
        strippedFilters.append(filterName.simplified());
    }
    return strippedFilters;
}

QString CoreHelper::findExtensioName(const QString &fileName, const QStringList &newNameFilters, QMimeDatabase *db)
{
    Q_ASSERT(db);
    QString newNameFilterExtension;

    const QString fileNameExtension = db->suffixForFileName(fileName);

    for (const QString &filter : newNameFilters) {   //从扩展名列表中轮询，目前发现的应用程序传入扩展名列表均只有一个
        newNameFilterExtension = db->suffixForFileName(filter);   //在QMimeDataBase里面查询扩展名是否存在（不能查询正则表达式）
        if (newNameFilterExtension.isEmpty()) {   //未查询到扩展名用正则表达式再查一次，新加部分，解决WPS保存文件去掉扩展名后没有补上扩展名的问题
            QRegExp regExp(filter.mid(2), Qt::CaseInsensitive, QRegExp::Wildcard);
            qInfo() << " Cannot find extesion name by QMimeDataBase::suffixForFileName，try regexp: " << filter;
            for (QMimeType m : db->allMimeTypes()) {
                for (QString suffixe : m.suffixes()) {
                    if (regExp.exactMatch(suffixe)) {
                        newNameFilterExtension = suffixe;
                        qInfo() << "Find extesion name by regexp: " << suffixe;
                        break;   //查询到后跳出循环
                    }
                }
                if (!newNameFilterExtension.isEmpty()) {
                    break;   //查询到后跳出循环
                }
            }
        }

        if (newNameFilterExtension.isEmpty())
            qInfo() << "Cannot find extension name";

        QRegExp re(newNameFilterExtension, Qt::CaseInsensitive, QRegExp::Wildcard);
        if (re.exactMatch(fileNameExtension)) {   //原扩展名与新扩展名不匹配？
            qInfo() << "Set new filter rules:" << newNameFilters;
            // TODO(liuyangming):
            // getFileView()->setNameFilters(newNameFilters); //这里传递回去的有可能是一个正则表达式，它决定哪些文件不被置灰
        }
    }
    return newNameFilterExtension;
}
