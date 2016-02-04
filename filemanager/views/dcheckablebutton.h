#ifndef DCHECKABLEBUTTON_H
#define DCHECKABLEBUTTON_H

#include <QPushButton>

class QLabel;

class DCheckableButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DCheckableButton(const QString& icon, const QString& text, QWidget *parent = 0);
    ~DCheckableButton();

    void initUI();

signals:

public slots:

private:
    QString m_icon;
    QString m_text;
    QLabel* m_iconLabel;
    QLabel* m_textLabel;
};

#endif // DCHECKABLEBUTTON_H
