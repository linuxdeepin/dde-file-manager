#ifndef DEDITORWIDGETMENU_H
#define DEDITORWIDGETMENU_H

#include <QLineEdit>
#include <QTextEdit>
#include <QPointer>

#include <DMenu>
#include <dwidget_global.h>

DWIDGET_USE_NAMESPACE

class DEditorWidgetMenu : public DMenu
{
    Q_OBJECT

public:
    explicit DEditorWidgetMenu(QLineEdit *parent);
    explicit DEditorWidgetMenu(QTextEdit *parent);

private:
    QPointer<QLineEdit> lineEdit;
    QPointer<QTextEdit> textEdit;

    void init(QWidget *obj);
};

#endif // DEDITORWIDGETMENU_H
