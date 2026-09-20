// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILETYPEMAPPER_H
#define FILETYPEMAPPER_H

#include "service_textindex_global.h"

#include <QHash>
#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class FileTypeMapper
{
public:
    static FileTypeMapper &instance();

    QString fileTypeForExtension(const QString &ext) const;
    QString fileTypeForPath(const QString &filePath) const;

private:
    FileTypeMapper();

    void loadMappings();

    QHash<QString, QString> m_extToType;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // FILETYPEMAPPER_H
