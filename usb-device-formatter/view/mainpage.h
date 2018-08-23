/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include "widgets/progressline.h"
#include <QVariantAnimation>

class MainPage : public QWidget
{
    Q_OBJECT

public:
    enum FsType {
        Fat16,
        Fat32,
        Ntfs,
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

    int getMaxLabelNameLength() const;
    void setMaxLabelNameLength(int maxLabelNameLength);

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
    QLabel* m_warnLabel = NULL;
    QString m_targetPath;
    QLabel* m_remainLabel = NULL;
    QLabel* m_nameLabel = NULL;
    int m_maxLabelNameLength = 0;
    QVariantAnimation* animator;
};

#endif // MAINPAGE_H
