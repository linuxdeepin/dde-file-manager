#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "widgets/progressline.h"

class MainPage : public QWidget
{
    Q_OBJECT

public:
    explicit MainPage(QWidget *parent = 0);
    void initUI();

signals:

public slots:

private:
    QLabel* m_iconLabel;
    QComboBox* m_typeCombo;
    QPushButton* m_formatButton;
    ProgressLine* m_sorageProgressBar;
    QStringList m_fileFormat;
    QString m_currentFormat;
};

#endif // MAINPAGE_H
