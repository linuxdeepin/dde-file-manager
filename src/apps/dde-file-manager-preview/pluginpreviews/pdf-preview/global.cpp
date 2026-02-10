// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global.h"
#include <dfm-base/mimetype/dmimedatabase.h>

#include <QMimeType>
#include <QDebug>
#include <DGuiApplicationHelper>

using namespace plugin_filepreview;
static QWidget *mainWidget = nullptr;

extern "C" FileType fileType(const QString &filePath)
{
    FileType fileType = FileType::kUnknown;

    DFMBASE_NAMESPACE::DMimeDatabase db;
    const QMimeType mimeType = db.mimeTypeForFile(QUrl::fromLocalFile(filePath), QMimeDatabase::MatchContent);

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
