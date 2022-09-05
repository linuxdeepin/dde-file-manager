// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JOBCONTROLLER_H
#define JOBCONTROLLER_H

#include "ddiriterator.h"

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QQueue>
#include <QDir>

QT_BEGIN_NAMESPACE
class QElapsedTimer;
QT_END_NAMESPACE

class JobController : public QThread
{
    Q_OBJECT

    Q_PROPERTY(int timeCeiling READ timeCeiling WRITE setTimeCeiling)
    Q_PROPERTY(int countCeiling READ countCeiling WRITE setCountCeiling)

public:
    enum State {
        Started,
        Paused,
        Stoped
    };

    explicit JobController(const DUrl &fileUrl, const DDirIteratorPointer &iterator, bool silent = false, QObject *parent = nullptr);
    explicit JobController(const DUrl &fileUrl, const QStringList &nameFilters,
                           QDir::Filters filters, bool silent = false, QObject *parent = nullptr);

    ~JobController() override;

    State state() const;

    int timeCeiling() const;
    int countCeiling() const;
    inline bool isUpdatedFinished() const {return m_updateFinished;}
public slots:
    void start();
    void pause();
    void stop();
    void stopAndDeleteLater();

    void setTimeCeiling(int timeCeiling);
    void setCountCeiling(int countCeiling);

signals:
    void stateChanged(State state);
    void addChildren(const DAbstractFileInfoPointer info, const bool isEnd = false);
    void addChildrenList(const QList<DAbstractFileInfoPointer> &infoList);
    void childrenUpdated(const QList<DAbstractFileInfoPointer> &list);

private:
    bool m_silent;
    DDirIteratorPointer m_iterator;
    DUrl m_fileUrl;
    QStringList m_nameFilters;
    QDir::Filters m_filters;

    State m_state = Stoped;
    QWaitCondition waitCondition;
    QMutex mutex;

    QElapsedTimer *timer = Q_NULLPTR;

    int m_timeCeiling = 100;
    int m_countCeiling = 3000;
    volatile bool m_updateFinished = true; //刷新完成标志

    void run() override;
    void setState(State state);
};

#endif // JOBCONTROLLER_H
