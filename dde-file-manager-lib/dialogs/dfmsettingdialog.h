#ifndef DFMSETTINGDIALOG_H
#define DFMSETTINGDIALOG_H


#include <DSettingsDialog>
#include <QCheckBox>

DWIDGET_USE_NAMESPACE

class DFMSettingDialog : public DSettingsDialog
{
public:
    DFMSettingDialog(QWidget *parent = 0);

    static QWidget *createAutoMountCheckBox(QObject* opt);
    static QWidget *createAutoMountOpenCheckBox(QObject* opt);

    static QCheckBox* AutoMountCheckBox;
    static QCheckBox* AutoMountOpenCheckBox;
};

#endif // DFMSETTINGDIALOG_H
