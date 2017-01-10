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
    enum FsType {
        Efi,
        Ext2,
        Ext3,
        Ext4,
        Fat16,
        Fat32,
        Hfsplus,
        Jfs,
        Linuxswap,
        Ntfs,
        Xfs
    };
    Q_ENUM(FsType)

    explicit MainPage(const QString& defautFormat = "", QWidget *parent = 0);
    void initUI();
    QString selectedFormat();
    void initConnections();
    QString getLabel();

    QString getTargetPath() const;
    void setTargetPath(const QString &targetPath);
    QString formatSize(const qint64& num);
    QString getSelectedFs() const;

signals:

public slots:
    void onCurrentSelectedTypeChanged(const QString& type);
protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QLabel* m_iconLabel = NULL;
    QComboBox* m_typeCombo = NULL;
    QPushButton* m_formatButton = NULL;
    ProgressLine* m_storageProgressBar = NULL;
    QStringList m_fileFormat;
    QString m_defautlFormat;
    QLineEdit* m_labelLineEdit = NULL;
    QString m_targetPath;
    QLabel* m_remainLabel = NULL;
    QLabel* m_warnLabel = NULL;
};

#endif // MAINPAGE_H
