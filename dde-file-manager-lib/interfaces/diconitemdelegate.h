#ifndef DFILEITEMDELEGATE_H
#define DFILEITEMDELEGATE_H

#include "dstyleditemdelegate.h"

#include <QPointer>

class FileIconItem;
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class DIconItemDelegatePrivate;
class DIconItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT

    Q_PROPERTY(QColor focusTextBackgroundBorderColor READ focusTextBackgroundBorderColor WRITE setFocusTextBackgroundBorderColor)
    Q_PROPERTY(bool enabledTextShadow READ enabledTextShadow WRITE setEnabledTextShadow)

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

    void updateItemSizeHint() Q_DECL_OVERRIDE;

    QColor focusTextBackgroundBorderColor() const;
    bool enabledTextShadow() const;

public slots:
    void setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor);
    void setEnabledTextShadow(bool enabledTextShadow);

protected:
    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    void onEditWidgetFocusOut();
    void onTriggerEdit(const QModelIndex &index);
    QSize iconSizeByIconSizeLevel() const;

    Q_DECLARE_PRIVATE(DIconItemDelegate)
};

#endif // DFILEITEMDELEGATE_H
