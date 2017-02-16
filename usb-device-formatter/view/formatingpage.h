#ifndef FORMATINGPAGE_H
#define FORMATINGPAGE_H

#include <QFrame>

class ProgressBox;

class FormatingPage : public QFrame
{
    Q_OBJECT
public:
    explicit FormatingPage(QWidget *parent = 0);
    void initUI();
    void animateToFinish(const bool& result);
    void startAnimate();

signals:
    void finished(const bool& successful);

public slots:
private:
    ProgressBox* m_progressBox = NULL;
};

#endif // FORMATINGPAGE_H
