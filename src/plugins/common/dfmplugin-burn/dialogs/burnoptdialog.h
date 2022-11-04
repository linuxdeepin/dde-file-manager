/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
    QCheckBox *donotcloseComb { nullptr };
    QLabel *postburnLabel { nullptr };
    QCheckBox *checkdiscCheckbox { nullptr };
    QCheckBox *ejectCheckbox { nullptr };
};

}   // namespace dfmplugin_burn

#endif   // BURNOPTDIALOG_H
