// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONVIEWMENU_H
#define COLLECTIONVIEWMENU_H

#include <DMenu>

#include <QObject>

namespace ddplugin_organizer {
class CollectionView;
class CollectionViewMenu : public QObject
{
    Q_OBJECT
public:
    explicit CollectionViewMenu(CollectionView *parent);
    static bool disableMenu();
    void emptyAreaMenu();
    void normalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags, const QPoint gridPos);
signals:

public slots:
protected:
    QWidget *getCanvasView();
private:
    CollectionView *view = nullptr;
    DTK_WIDGET_NAMESPACE::DMenu *menuPtr { nullptr };
};
}

#endif // COLLECTIONVIEWMENU_H
