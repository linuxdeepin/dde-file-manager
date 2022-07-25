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

#include "global.h"
#include "dfm-base/mimetype/dmimedatabase.h"

#include <QMimeType>
#include <QDebug>
#include <DGuiApplicationHelper>

using namespace plugin_filepreview;
static QWidget *mainWidget = nullptr;

extern "C" FileType fileType(const QString &filePath)
{
    FileType fileType = FileType::kUnknown;

    DFMBASE_NAMESPACE::DMimeDatabase db;
    const QMimeType mimeType = db.mimeTypeForFile(filePath, QMimeDatabase::MatchContent);

    if (mimeType.name() == QLatin1String("application/pdf")) {
        fileType = kPDF;
    }

    return fileType;
}

extern "C" void setMainWidget(QWidget *pMainWidget)
{
    mainWidget = pMainWidget;
}

extern "C" QWidget *getMainDialog()
{
    return mainWidget;
}
