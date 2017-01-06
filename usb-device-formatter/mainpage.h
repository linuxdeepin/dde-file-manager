#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include "widgets/progressline.h"

class MainPage : public QWidget
{
    Q_OBJECT

public:

    explicit MainPage(const QString& defautFormat = "", QWidget *parent = 0);
    void initUI();
    QString selectedFormat();
    void initConnections();
    QString getLabel();

    QString getTargetPath() const;
    void setTargetPath(const QString &targetPath);
    QString formatSize(const qint64& num);

signals:

public slots:
    void onCurrentSelectedTypeChanged(const QString& type);

private:
    QLabel* m_iconLabel;
    QComboBox* m_typeCombo;
    QPushButton* m_formatButton;
    ProgressLine* m_storageProgressBar;
    QStringList m_fileFormat;
    QString m_defautlFormat;
    QLineEdit* m_labelLineEdit;
    QString m_targetPath;
    QLabel* m_remainLabel;
};

#endif // MAINPAGE_H
