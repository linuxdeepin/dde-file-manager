#ifndef DICONTEXTBUTTON_H
#define DICONTEXTBUTTON_H

#include <QPushButton>

class DIconTextButton : public QPushButton
{
    Q_OBJECT
public:
    DIconTextButton(const QIcon& icon, const QString &text, QWidget *parent=0);
    ~DIconTextButton();

signals:

public slots:
};

#endif // DICONTEXTBUTTON_H
