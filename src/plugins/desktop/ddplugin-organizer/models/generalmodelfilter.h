// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GENERALMODELFILTER_H
#define GENERALMODELFILTER_H

#include "modeldatahandler.h"

#include <QSharedPointer>

namespace ddplugin_organizer {

class GeneralModelFilter : public ModelDataHandler
{
public:
    explicit GeneralModelFilter();
    bool installFilter(const QSharedPointer<ModelDataHandler> &filter);
    void removeFilter(const QSharedPointer<ModelDataHandler> &filter);
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
    bool acceptUpdate(const QUrl &url, const QVector<int> &roles = {}) override;
protected:
    QList<QSharedPointer<ModelDataHandler>> modelFilters;
};

}

#endif // GENERALMODELFILTER_H
