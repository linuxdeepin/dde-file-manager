// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MASTEREDMEDIAFILEINFOPRIVATE_H
#define MASTEREDMEDIAFILEINFOPRIVATE_H
#include "dfmplugin_optical_global.h"
#include "dfm-base/interfaces/private/abstractfileinfo_p.h"
namespace dfmplugin_optical {
class MasteredMediaFileInfo;
class MasteredMediaFileInfoPrivate : public dfmbase::AbstractFileInfoPrivate
{
    friend class MasteredMediaFileInfo;

public:
    explicit MasteredMediaFileInfoPrivate(const QUrl &url, MasteredMediaFileInfo *qq);
    virtual ~MasteredMediaFileInfoPrivate();

private:
    void backupInfo(const QUrl &url);
    QUrl parentUrl() const;
    bool canDrop();

private:
    QUrl backerUrl;
    QString curDevId;
    QVariantMap devInfoMap;
};
}
#endif   // MASTEREDMEDIAFILEINFOPRIVATE_H
