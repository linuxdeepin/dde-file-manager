#ifndef FILEITEM_H
#define FILEITEM_H

#include <QLabel>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

class FileIconItem : public QFrame
{
    Q_OBJECT

public:
    explicit FileIconItem(QWidget *parent = 0);

    void setOpacity(qreal opacity);

    QSize sizeHint() const Q_DECL_OVERRIDE;

    inline QLabel* getIconLabel() const
    { return icon; }
    inline QTextEdit* getTextEdit() const
    { return edit; }

signals:
    void inputFocusOut();

protected:
    bool event(QEvent *ee) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *ee) Q_DECL_OVERRIDE;

private:
    void updateEditorGeometry();

    bool canDeferredDelete = true;
    QLabel *icon;
    QTextEdit *edit;
    QGraphicsOpacityEffect *opacityEffect = Q_NULLPTR;

    friend class DIconItemDelegate;
    friend class DFileView;
    friend class DFileViewHelper;
};

#endif // FILEITEM_H
