/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef PREVIEWDIALOGMANAGER_H
#define PREVIEWDIALOGMANAGER_H

#include "dfmplugin_filepreview_global.h"
#include "services/filemanager/windows/windowsservice.h"
#include "views/filepreviewdialog.h"

#include <QObject>

DPFILEPREVIEW_BEGIN_NAMESPACE
class PreviewDialogManager : public QObject
{
    Q_OBJECT
public:
    static PreviewDialogManager *instance();

private:
    explicit PreviewDialogManager(QObject *parent = nullptr);

signals:

public:
    DSB_FM_NAMESPACE::WindowsService *windowService();

public slots:
    void showPreviewDialog(const quint64 winId, const QList<QUrl> &selecturls, const QList<QUrl> dirUrl);

private:
    FilePreviewDialog *filePreviewDialog { nullptr };
};
DPFILEPREVIEW_END_NAMESPACE
#endif   // PREVIEWDIALOGMANAGER_H
