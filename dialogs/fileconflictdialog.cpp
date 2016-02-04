#include "fileconflictdialog.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QLayout>
#include <QLabel>
#include <QPushButton>

FileConflictDialog::FileConflictDialog(QWidget *parent) : DBaseDialog(parent)
{
    QString icon = ":/images/dialogs/images/user-trash-full.png";
    QString message = "Are you want to replace this item?";
    QString tipMessage = "There exists a file with same name!";
    QStringList buttons;
    buttons << "Cancel" << "Replace" << "Skip";
    initUI(icon, message, tipMessage, buttons, buttons);
    moveCenter();
    getButtonsGroup()->button(1)->setFocus();
}

void FileConflictDialog::addConflictFiles(QString newFile, QString oldFile){
    QFileInfo newInfo(newFile);
    QFileInfo oldInfo(oldFile);

    QString message = QString("Are you want to replace %1?").arg(newInfo.fileName());
    setMessage(message);

    QString pathBaseName = QFileInfo(newInfo.path()).baseName();
    QString tipMessage = QString("In") + QString("\" %1 \"").arg(pathBaseName) +
            QString("exists files in same name") +  QString("%2").arg(newInfo.baseName());
    setTipMessage(tipMessage);
}


QHBoxLayout* FileConflictDialog::createFileLayout(QString icon, QString size,
                                                  QString time, QString fileTip){
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setFixedSize(48, 48);
    iconLabel->setPixmap(QPixmap(icon).scaled(iconLabel->size()));
    QLabel* primarylabel = new QLabel(fileTip, this);
    QLabel* sizeLabel = new QLabel(size, this);
    QLabel* timeLabel = new QLabel(time, this);

    QHBoxLayout* iconLayout = new QHBoxLayout;
    iconLayout->addWidget(iconLabel, Qt::AlignCenter);

    QVBoxLayout* messageLayout = new QVBoxLayout;
    messageLayout->addWidget(primarylabel);
    messageLayout->addWidget(sizeLabel);
    messageLayout->addWidget(timeLabel);
    return iconLayout;
}


FileConflictDialog::~FileConflictDialog()
{

}

