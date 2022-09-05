// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHSERVICE_P_H
#define SEARCHSERVICE_P_H

#include "searchservice.h"
#include "maincontroller/maincontroller.h"

class SearchServicePrivate : public QObject
{
    Q_OBJECT
    friend class SearchService;

public:
    explicit SearchServicePrivate(SearchService *parent);
    ~SearchServicePrivate();

private:
    MainController *mainController = nullptr;
};

#endif   // SEARCHSERVICE_P_H
