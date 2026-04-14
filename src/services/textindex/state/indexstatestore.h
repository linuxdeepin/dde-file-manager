// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INDEXSTATESTORE_H
#define INDEXSTATESTORE_H

#include "profile/indexprofile.h"
#include "utils/indexutility.h"

#include <QDateTime>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexStateStore
{
public:
    explicit IndexStateStore(IndexProfile profile);

    QString statusFilePath() const;

    IndexUtility::IndexState getIndexState() const;
    void setIndexState(IndexUtility::IndexState state) const;
    bool isCleanState() const;

    bool needsRebuild() const;
    void setNeedsRebuild(bool need) const;

    QString getLastUpdateTime() const;
    int getIndexVersion() const;
    bool isCompatibleVersion() const;

    void removeIndexStatusFile() const;
    void clearIndexDirectory() const;
    void saveIndexStatus(const QDateTime &lastUpdateTime) const;
    void saveIndexStatus(const QDateTime &lastUpdateTime, int version) const;

    // Save last update time only, without updating version
    // Used by incremental tasks which should not change the version number
    void saveLastUpdateTime(const QDateTime &lastUpdateTime) const;

private:
    IndexProfile m_profile;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // INDEXSTATESTORE_H
