// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLLECTIONITEMDELEGATE_P_H
#define COLLECTIONITEMDELEGATE_P_H

#include "collectionitemdelegate.h"

#include <dfm-base/utils/elidetextlayout.h>

#include <QPointer>
#include <QTextDocument>
#include <QAbstractItemView>

namespace ddplugin_organizer {

class CollectionItemDelegatePrivate
{
public:
    explicit CollectionItemDelegatePrivate(CollectionItemDelegate *qq);
    ~CollectionItemDelegatePrivate();

    dfmbase::ElideTextLayout *createTextlayout(const QModelIndex &index, const QPainter *painter = nullptr) const;

    inline QRect availableTextRect(QRect labelRect) const {
        // available text rect top is label rect minus icon space and text padding.
        labelRect.setTop(labelRect.top() + CollectionItemDelegate::kIconSpacing
                    + CollectionItemDelegate::kTextPadding);
        return labelRect;
    }
    inline bool isHighlight(const QStyleOptionViewItem &option) const {
        return (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    }

    bool needExpend(const QStyleOptionViewItem &option,
                    const QModelIndex &index, const QRect &rText, QRect *needText = nullptr) const;

    static void extendLayoutText(const FileInfoPointer &info, dfmbase::ElideTextLayout *layout);
public:
    // default icon size is 48px.
    int currentIconLevel = -1;
    int textLineHeight = -1;
    static const QList<int> kIconSizes;
    //QList<int> charOfLine;
    QStringList iconLevelDescriptions;
    QSize itemSizeHint;

    QTextDocument *document { nullptr };
    CollectionItemDelegate *const q = nullptr;
};

}

#endif   // COLLECTIONITEMDELEGATE_P_H
