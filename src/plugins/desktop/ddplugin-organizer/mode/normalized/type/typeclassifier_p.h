// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPECLASSIFIER_P_H
#define TYPECLASSIFIER_P_H

#include "typeclassifier.h"

namespace ddplugin_organizer {

class TypeClassifierPrivate
{
public:
    explicit TypeClassifierPrivate(TypeClassifier *qq);
    ~TypeClassifierPrivate();

public:
    ItemCategories categories;
    const QHash<QString, QString> keyNames;
    const QSet<QString> docSuffix;
    const QSet<QString> picSuffix;
    const QSet<QString> muzSuffix;
    const QSet<QString> vidSuffix;
    const QSet<QString> appSuffix;
    //const QSet<QString> appMimeType;
private:
    TypeClassifier *q;
};

}

#endif   // TYPECLASSIFIER_P_H
