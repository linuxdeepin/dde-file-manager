/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TYPECLASSIFIER_H
#define TYPECLASSIFIER_H

#include "mode/normalized/fileclassifier.h"

#include <QSet>

namespace ddplugin_organizer {

class TypeClassifierPrivate;
class TypeClassifier : public FileClassifier
{
    Q_OBJECT
    friend class TypeClassifierPrivate;
public:
    explicit TypeClassifier(QObject *parent = nullptr);
    ~TypeClassifier();
    Classifier mode() const override;
    ModelDataHandler *dataHandler() const override;
    QStringList classes() const override;
    QString classify(const QUrl &) const override;
    QString className(const QString &key) const override;
private:
    TypeClassifierPrivate *d;
    ModelDataHandler *handler = nullptr;
};
}

#endif // TYPECLASSIFIER_H
