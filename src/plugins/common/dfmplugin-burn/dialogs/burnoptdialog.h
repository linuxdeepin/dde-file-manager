// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BURNOPTDIALOG_H
#define BURNOPTDIALOG_H

#include "dfmplugin_burn_global.h"

#include <dfm-burn/dburn_global.h>

#include <DDialog>
#include <DLineEdit>
#include <DCommandLinkButton>
#include <QUrl>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>

namespace dfmplugin_burn {

class BurnOptDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit BurnOptDialog(const QString &dev, QWidget *parent = nullptr);

    void setUDFSupported(bool supported, bool disableISOOpts);
    void setISOImage(const QUrl &image);
    void setDefaultVolName(const QString &volName);
    void setWriteSpeedInfo(const QStringList &writespeed);

private:
    void initializeUi();
    void initConnect();
    DFMBURN::BurnOptions currentBurnOptions();
    void startDataBurn();
    void startImageBurn();

private slots:
    void onIndexChanged(int index);
    void onButnBtnClicked(int index, const QString &text);

private:
    QString curDev;
    QHash<QString, int> speedMap;
    QUrl imageFile;
    bool isSupportedUDF { false };
    QString lastVolName;

    DTK_WIDGET_NAMESPACE::DCommandLinkButton *advanceBtn { nullptr };
    QWidget *advancedSettings { nullptr };
    QWidget *content { nullptr };
    QLabel *volnameLabel { nullptr };
    QLineEdit *volnameEdit { nullptr };
    QLabel *writespeedLabel { nullptr };
    QComboBox *writespeedComb { nullptr };
    QLabel *fsLabel { nullptr };
    QComboBox *fsComb { nullptr };
    QCheckBox *finalizeDiscCheckbox { nullptr };
    QLabel *postburnLabel { nullptr };
    QCheckBox *checkdiscCheckbox { nullptr };
    QCheckBox *ejectCheckbox { nullptr };
};

}   // namespace dfmplugin_burn

#endif   // BURNOPTDIALOG_H
