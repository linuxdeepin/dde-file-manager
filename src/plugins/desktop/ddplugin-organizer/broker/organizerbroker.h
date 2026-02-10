// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORGANIZERBROKER_H
#define ORGANIZERBROKER_H

#include <QObject>
#include <QPoint>
#include <QRect>

class QAbstractItemView;
namespace ddplugin_organizer {

class OrganizerBroker : public QObject
{
    Q_OBJECT
public:
    explicit OrganizerBroker(QObject *parent = nullptr);
    ~OrganizerBroker() override;
    virtual bool init();

signals:

public slots:
    virtual void refreshModel(bool global, int ms, bool file) = 0;
    virtual QString gridPoint(const QUrl &item, QPoint *point) = 0;
    virtual QRect visualRect(const QString &id, const QUrl &item) = 0;
    virtual QAbstractItemView * view(const QString &id) = 0;
    virtual QRect iconRect(const QString &id, QRect vrect) = 0;
    virtual bool selectAllItems() = 0;
};
}

#endif // ORGANIZERBROKER_H
