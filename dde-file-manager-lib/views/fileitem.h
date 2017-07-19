#ifndef FILEITEM_H
#define FILEITEM_H

#include <QLabel>
#include <QStack>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

class FileIconItem : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)

public:
    explicit FileIconItem(QWidget *parent = 0);

    void setOpacity(qreal opacity);

    QSize sizeHint() const Q_DECL_OVERRIDE;

    inline QLabel* getIconLabel() const
    { return icon; }
    inline QTextEdit* getTextEdit() const
    { return edit; }

    QColor borderColor() const;

public slots:
    void setBorderColor(QColor borderColor);

signals:
    void inputFocusOut();
    void borderColorChanged(QColor borderColor);

private slots:
    void popupEditContentMenu();
    void editUndo();
    void editRedo();

protected:
    void updateEditorGeometry();
    bool event(QEvent *ee) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *ee) Q_DECL_OVERRIDE;

private:
    void updateStyleSheet();

    QString editTextStackCurrentItem() const;
    QString editTextStackBack();
    QString editTextStackAdvance();
    void pushItemToEditTextStack(const QString &item);

    bool canDeferredDelete = true;
    QLabel *icon;
    QTextEdit *edit;
    int editTextStackCurrentIndex = -1;
    bool disableEditTextStack = false;
    QStack<QString> editTextStack;
    QGraphicsOpacityEffect *opacityEffect = Q_NULLPTR;
    QColor m_borderColor;

    friend class DIconItemDelegate;
    friend class DFileView;
    friend class DFileViewHelper;
};

#endif // FILEITEM_H
