// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DELETEJOBCONTROLLER_H
#define DELETEJOBCONTROLLER_H

#include <QObject>

#include <QtCore>
#include <QtDBus>

class DeleteJobInterface;

class DeleteJobController : public QObject
{
    Q_OBJECT
public:
    explicit DeleteJobController(QObject *parent = 0);
    ~DeleteJobController();

    void initConnect();

signals:

public slots:
    void createDeleteJob(const QStringList& files);
};

#endif // DELETEJOBCONTROLLER_H
