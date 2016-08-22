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

    bool isVisible() const;
    void setIsVisible(bool isVisible);

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    QPointer<QLineEdit> lineEdit;
    QPointer<QTextEdit> textEdit;

    bool m_isVisible = false;

    void init(QWidget *obj);
};

#endif // DEDITORWIDGETMENU_H
