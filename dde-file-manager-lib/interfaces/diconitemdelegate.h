#ifndef DFILEITEMDELEGATE_H
#define DFILEITEMDELEGATE_H

#include "dstyleditemdelegate.h"

#include <QPointer>

class FileIconItem;
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class DIconItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DIconItemDelegate(DFileViewHelper *parent);
    ~DIconItemDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const Q_DECL_OVERRIDE;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex &) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget * editor, const QModelIndex & index) const Q_DECL_OVERRIDE;

    void paintIconItem(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index, bool isDragMode, bool isActive) const;

    QList<QRect> paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    QModelIndexList hasWidgetIndexs() const Q_DECL_OVERRIDE;
    void hideNotEditingIndexWidget() Q_DECL_OVERRIDE;

    QModelIndex expandedIndex() const;
    FileIconItem *expandedIndexWidget() const;

    int iconSizeLevel() const Q_DECL_OVERRIDE;
    int minimumIconSizeLevel() const Q_DECL_OVERRIDE;
    int maximumIconSizeLevel() const Q_DECL_OVERRIDE;

    int increaseIcon() Q_DECL_OVERRIDE;
    int decreaseIcon() Q_DECL_OVERRIDE;
    int setIconSizeByIconSizeLevel(int level) Q_DECL_OVERRIDE;

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    QPointer<FileIconItem> expanded_item;

    mutable QHash<QString, QString> m_elideMap;
    mutable QHash<QString, QString> m_wordWrapMap;
    mutable QHash<QString, int> m_textHeightMap;
    mutable QHash<QString, QTextDocument*> m_documentMap;
    mutable QModelIndex expanded_index;
    mutable QModelIndex lastAndExpandedInde;

    QList<int> m_iconSizes;
    /// default icon size is 64px.
    int m_currentIconSizeIndex = 1;

    void onEditWidgetFocusOut();
    void onIconSizeChanged();
    void onTriggerEdit(const QModelIndex &index);
    QSize iconSizeByIconSizeLevel() const;
};

#endif // DFILEITEMDELEGATE_H
