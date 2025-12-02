// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corehelper.h"
#include "views/filedialog.h"

#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfmio_utils.h>

#include <DDialog>
#include <DPlatformWindowHandle>

#include <QObject>
#include <QLabel>
#include <QRegularExpression>

using namespace filedialog_core;
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

/*!
 * \brief workspace must exist when invoke some interfaces
 * \param func
 */
void CoreHelper::delayInvokeProxy(std::function<void()> func, quint64 winID, QObject *parent)
{
    auto window = qobject_cast<FileDialog *>(FMWindowsIns.findWindowById(winID));
    Q_ASSERT(window);

    if (window->workSpace()) {
        func();
    } else {
        QObject::connect(window, &FileDialog::initialized, parent, [func]() {
            func();
        });
    }
}

/*!
 * \brief Files with filenames starting with a dot are considered as hidden files and need to be checked
 * \return true if don't save as hidden file
 */
bool CoreHelper::askHiddenFile(QWidget *parent)
{
    DDialog dialog(parent);

    dialog.setIcon(QIcon::fromTheme("dde-file-manager"));
    dialog.setTitle(QObject::tr("This file will be hidden if the file name starts with '.'. Do you want to hide it?"));
    dialog.addButton(QObject::tr("Hide", "button"), false, DDialog::ButtonWarning);
    dialog.addButton(QObject::tr("Cancel", "button"), true);

    return dialog.exec() != 0;
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

    dialog.setIcon(QIcon::fromTheme("dde-file-manager"));

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
    QRegularExpression r(QString::fromLatin1("^(.*)\\(([^()]*)\\)$"));
    const int numFilters = filters.count();
    strippedFilters.reserve(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        QString filterName = filters[i];
        QRegularExpressionMatch match = r.match(filterName);
        if (match.hasMatch()) {
            filterName = match.captured(1);
        }
        strippedFilters.append(filterName.simplified());
    }
    return strippedFilters;
}

QString CoreHelper::findExtensionName(const QString &fileName, const QStringList &newNameFilters, QMimeDatabase *db)
{
    Q_ASSERT(db);
    QString newNameFilterExtension;

    const QString fileNameExtension = db->suffixForFileName(fileName);

    for (const QString &filter : newNameFilters) {   //从扩展名列表中轮询，目前发现的应用程序传入扩展名列表均只有一个
        newNameFilterExtension = db->suffixForFileName(filter);   //在QMimeDataBase里面查询扩展名是否存在（不能查询正则表达式）
        if (newNameFilterExtension.isEmpty()) {   //未查询到扩展名用正则表达式再查一次，新加部分，解决WPS保存文件去掉扩展名后没有补上扩展名的问题
            QRegularExpression regExp(QRegularExpression::wildcardToRegularExpression(filter.mid(2)),
                                      QRegularExpression::CaseInsensitiveOption);
            fmInfo() << "File Dialog: Cannot find extesion name by QMimeDataBase::suffixForFileName，try regexp: " << filter;
            for (QMimeType m : db->allMimeTypes()) {
                for (QString suffixe : m.suffixes()) {
                    if (regExp.match("^" + suffixe + "$").hasMatch()) {
                        newNameFilterExtension = suffixe;
                        fmInfo() << "Find extesion name by regexp: " << suffixe;
                        break;   //查询到后跳出循环
                    }
                }
                if (!newNameFilterExtension.isEmpty()) {
                    break;   //查询到后跳出循环
                }
            }
        }

        QRegularExpression re(QRegularExpression::wildcardToRegularExpression(newNameFilterExtension),
                              QRegularExpression::CaseInsensitiveOption);
        if (re.match("^" + fileNameExtension + "$").hasMatch()) {   //原扩展名与新扩展名不匹配？
            fmInfo() << "Set new filter rules:" << newNameFilters;
            // TODO(liuyangming):
            // getFileView()->setNameFilters(newNameFilters); //这里传递回去的有可能是一个正则表达式，它决定哪些文件不被置灰
        }

        if (!newNameFilterExtension.isEmpty())
            break;
    }
    return newNameFilterExtension;
}
