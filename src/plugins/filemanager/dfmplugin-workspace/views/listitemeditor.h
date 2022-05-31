#ifndef LISTITEMEDITOR_H
#define LISTITEMEDITOR_H

#include "dfmplugin_workspace_global.h"

#include <DArrowRectangle>

#include <QLineEdit>

DPWORKSPACE_BEGIN_NAMESPACE

class ListItemEditor : public QLineEdit
{
    Q_OBJECT
public:
    explicit ListItemEditor(QWidget *parent = nullptr);
    ~ListItemEditor();
    void select(const QString &part);
    inline void setMaxCharSize(int l)
    {
        if (l > 0)
            theMaxCharSize = l;
    }

    inline int maxCharSize() const
    {
        return theMaxCharSize;
    }

signals:
    void inputFocusOut();

public slots:
    void showAlertMessage(const QString &text, int duration = 3000);

protected:
    static DTK_WIDGET_NAMESPACE::DArrowRectangle *createTooltip();

    bool event(QEvent *ee) override;

private slots:
    void onEditorTextChanged(const QString &text);

private:
    void init();
    bool processLength(QString &text, int &pos);

private:
    int theMaxCharSize = INT_MAX;
    DTK_WIDGET_NAMESPACE::DArrowRectangle *tooltip = nullptr;
};
DPWORKSPACE_END_NAMESPACE
#endif   // LISTITEMEDITOR_H
