// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NORMALIZEDMODEBROKER_H
#define NORMALIZEDMODEBROKER_H

#include "broker/organizerbroker.h"

namespace ddplugin_organizer {
class NormalizedMode;
class CollectionView;
class NormalizedModeBroker : public OrganizerBroker
{
    Q_OBJECT
public:
    explicit NormalizedModeBroker(NormalizedMode *parent = nullptr);
public slots:
    void refreshModel(bool global, int ms, bool file) override;
    QString gridPoint(const QUrl &item, QPoint *point) override;
    QRect visualRect(const QString &id, const QUrl &item) override;
    QAbstractItemView *view(const QString &id) override;
    QRect iconRect(const QString &id, QRect vrect) override;
    bool selectAllItems() override;
private:
    NormalizedMode *mode;
};

}

#endif // NORMALIZEDMODEBROKER_H
