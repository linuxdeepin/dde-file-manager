#ifndef FORMATINGPAGE_H
#define FORMATINGPAGE_H

#include <QFrame>

class FormatingPage : public QFrame
{
    Q_OBJECT
public:
    explicit FormatingPage(QWidget *parent = 0);
    void initUI();

signals:
    void finished(const bool& successful);

public slots:
};

#endif // FORMATINGPAGE_H
