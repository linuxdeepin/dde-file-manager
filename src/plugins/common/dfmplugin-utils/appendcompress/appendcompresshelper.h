// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPENDCOMPRESSHELPER_H
#define APPENDCOMPRESSHELPER_H

#include "dfmplugin_utils_global.h"

#include <QObject>
#include <QUrl>

namespace dfmplugin_utils {

class AppendCompressHelper : public QObject
{
    Q_OBJECT
public:
    static bool setMouseStyle(const QUrl &toUrl, const QList<QUrl> &fromUrls, Qt::DropAction *dropAction);
    static bool dragDropCompress(const QUrl &toUrl, const QList<QUrl> &fromUrls);
    static bool isCompressedFile(const QUrl &toUrl);

private:
    explicit AppendCompressHelper(QObject *parent = nullptr);
    static bool appendCompress(const QString &toFilePath, const QStringList &fromFilePaths);
    static bool canAppendCompress(const QList<QUrl> &fromUrls, const QUrl &toUrl);
};

}

#endif   // APPENDCOMPRESSHELPER_H
