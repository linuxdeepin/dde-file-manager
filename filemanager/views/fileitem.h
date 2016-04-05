#ifndef FILEITEM_H
#define FILEITEM_H

#include <QLabel>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

class FileIconItem : public QFrame
{
    Q_OBJECT

public:
    explicit FileIconItem(QWidget *parent = 0);

protected:
    bool event(QEvent *ee) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *ee) Q_DECL_OVERRIDE;

private:
    bool canDeferredDelete = true;
    QLabel *icon;
    QTextEdit *edit;

    friend class DFileItemDelegate;
    friend class DFileView;
};

#endif // FILEITEM_H
