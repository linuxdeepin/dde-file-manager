/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "templateaction.h"

#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/utils/actiontypemanager.h"

#include <QDir>
#include <QStandardPaths>

DSC_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE

TemplateAction::TemplateAction(QObject *parent)
    : QObject(parent)
{
}

static void onActionTriggered(QAction *action)
{
    Q_UNUSED(action)
    //  TODO(lee)
    qDebug() << "TemplateAction --> onActionTriggered";
}

QList<ActionDataContainer> TemplateAction::getTemplateFileList()
{
    QList<ActionDataContainer> result;
    // blumia: Following is support for `xdg-user-dir TEMPLATES` Templates folder.
    //         It's suppored by many GNOME Nautilus based file manager. I don't think it's a good idea
    //         since we can't localization the file name text at all.
    // blumia: templateFolderPathCStr owned by glib, should NOT be freeed.

    // TODO(lee待确认)不知道后续是否有接口提供(临时组装)
    QString templateFolderPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/.Templates";

    if (!templateFolderPath.isEmpty()) {
        QDir templateFolder(templateFolderPath);
        if (templateFolder.exists() && templateFolder != QDir::home()) {   // accroding to xdg-user-dir, dir point to home means disable.
            const QFileInfoList &templateFileInfoList = templateFolder.entryInfoList(QDir::Files | QDir::Readable | QDir::NoSymLinks);
            int i = 1;
            for (const QFileInfo &fileInfo : templateFileInfoList) {
                QString errString;
                QUrl ttt = QUrl::fromLocalFile(fileInfo.filePath());
                auto fileinfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(QUrl::fromLocalFile(fileInfo.filePath()), true, &errString);
                if (!fileinfo) {
                    qInfo() << "getTemplateFileList create LocalFileInfo error: " << errString;
                    continue;
                }
                const QString entrySourcePath = fileInfo.absoluteFilePath();
                const QString entryText = fileInfo.baseName();
                const QString entryFileBaseName = entryText;   // suffix is based on source file, only base name is okay here.
                QIcon icon = fileinfo->fileIcon();

                auto actDc = ActionTypeManager::instance().registerActionType(QString("NewDocumentSub_%1").arg(i), entryText);
                actDc.second.setIcon(icon);
                actDc.second.setData(QVariant::fromValue(qMakePair(entrySourcePath, entryFileBaseName)));
                actDc.second.registerTriggeredFunc([](QAction *act) {
                    onActionTriggered(act);
                });
                result << actDc.second;
                ++i;
            }
        }
    }
    return result;
}

TemplateAction &TemplateAction::instance()
{
    static TemplateAction ins;
    return ins;
}

DSC_END_NAMESPACE
