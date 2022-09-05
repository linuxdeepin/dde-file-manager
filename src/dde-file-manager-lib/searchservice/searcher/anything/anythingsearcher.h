// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANYTHINGSEARCH_H
#define ANYTHINGSEARCH_H

#include "abstractsearcher.h"

#include <QTime>
#include <QMutex>

class ComDeepinAnythingInterface;
class AnythingSearcher : public AbstractSearcher
{
    Q_OBJECT
    friend class TaskCommander;
    friend class TaskCommanderPrivate;

private:
    struct SearchInfo
    {
        bool hasSymLink = false;
        QString realSearchPath;     // 真实的搜索路径
        QString symLinkPart;        // 记录链接路径
        QString symLinkTarget;      // 记录链接指向的路径
    };

    explicit AnythingSearcher(const DUrl &url, const QString &keyword, bool dataFlag, QObject *parent = nullptr);
    virtual ~AnythingSearcher() override;

    static bool isSupported(const DUrl &url, bool &isPrependData);
    bool search() override;
    void stop() override;
    bool hasItem() const override;
    QList<DUrl> takeAll() override;
    void tryNotify();
    bool hasSymLinkDirInSearchPath(const QString &path, SearchInfo &info);

private:
    ComDeepinAnythingInterface *anythingInterface = nullptr;
    QAtomicInt status = kReady;
    QList<DUrl> allResults;
    mutable QMutex mutex;
    bool isPrependData;

    //计时
    QTime notifyTimer;
    int lastEmit = 0;
};

#endif   // ANYTHINGSEARCH_H
