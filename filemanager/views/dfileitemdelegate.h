#ifndef DFILEITEMDELEGATE_H
#define DFILEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QHeaderView>

#include "dfileview.h"

DWIDGET_USE_NAMESPACE

class FileIconItem;

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class DFileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DFileItemDelegate(DFileView *parent = 0);
    ~DFileItemDelegate();

    inline DFileView *parent() const
    {
        return qobject_cast<DFileView*>(QStyledItemDelegate::parent());
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const Q_DECL_OVERRIDE;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget * editor, const QModelIndex & index) const Q_DECL_OVERRIDE;

    void paintIconItem(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    void paintListItem(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    QList<QRect> paintGeomertyss(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void hideAllIIndexWidget();
    void commitDataAndCloseActiveEditor();

    QModelIndex editingIndex() const;

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    FileIconItem *focus_item;

    mutable QHash<QString, QString> m_elideMap;
    mutable QHash<QString, QString> m_wordWrapMap;
    mutable QHash<QString, int> m_textHeightMap;
    mutable QHash<QString, QTextDocument*> m_documentMap;
    mutable QModelIndex focus_index;
    mutable QModelIndex editing_index;

    friend class DFileView;
};

#endif // DFILEITEMDELEGATE_H
