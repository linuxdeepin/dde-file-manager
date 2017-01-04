#include "formatingpage.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include "widgets/progressbox.h"

FormatingPage::FormatingPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void FormatingPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    ProgressBox* progressBox = new ProgressBox(this);
    progressBox->setFixedSize(100, 100);
    QLabel* formatingLabel = new QLabel(this);
    QString formatingText = tr("Formating,please wait for a minute...");
    formatingLabel->setText(formatingText);

    connect(progressBox, &ProgressBox::finished, this, &FormatingPage::finished);

    mainLayout->addStretch(1);
    mainLayout->addWidget(progressBox, 0 , Qt::AlignHCenter);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(formatingLabel, 0 , Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}
