// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONVIEWBROKER_H
#define COLLECTIONVIEWBROKER_H

#include <QObject>
#include <QRect>

namespace ddplugin_organizer {

class CollectionView;
class CollectionViewBroker : public QObject
{
    Q_OBJECT
public:
    explicit CollectionViewBroker(CollectionView *parent = nullptr);
    inline CollectionView *getView() const {return view;}
    void setView(CollectionView *v);
    bool gridPoint(const QUrl &file, QPoint &pos) const;
    QRect visualRect(const QUrl &file) const;
signals:

public slots:
private:
    CollectionView *view = nullptr;
};

}

#endif // COLLECTIONVIEWBROKER_H
