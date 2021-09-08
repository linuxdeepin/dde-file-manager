#ifndef COMPLETERVIEW_P_H
#define COMPLETERVIEW_P_H

#include "dfm_filemanager_service_global.h"

#include <QObject>
#include <QCompleter>
#include <QListView>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFileSystemModel>

DSB_FM_BEGIN_NAMESPACE

class CompleterViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CompleterViewDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

class CompleterView;
class CompleterViewPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(CompleterView)
    CompleterView *const q_ptr;

    QCompleter completer;
    QStringListModel model;
    CompleterViewDelegate delegate;

    explicit CompleterViewPrivate(CompleterView *qq);
};

DSB_FM_END_NAMESPACE

#endif // COMPLETERVIEW_P_H
