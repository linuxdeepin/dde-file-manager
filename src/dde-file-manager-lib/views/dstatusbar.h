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

#ifndef DSTATUSBAR_H
#define DSTATUSBAR_H

#include <dpicturesequenceview.h>
#include <dslider.h>

#include <QStatusBar>
#include <QHBoxLayout>
#include <QLabel>
#include <QSizeGrip>
#include <QPair>

#include "durl.h"
#include "dfmglobal.h"

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class DFMEvent;

DFM_BEGIN_NAMESPACE
class DFileStatisticsJob;
DFM_END_NAMESPACE

DFM_USE_NAMESPACE


class DFMElidLabel : public QLabel
{
public:
    explicit DFMElidLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(parent, f)
    {
    }
    explicit DFMElidLabel(const QString &text, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
        : QLabel(text, parent, f)
    {

    }
    ~DFMElidLabel() {}

    void setText(const QString &text);
protected:
    void setElidText(const QString &text);
    void resizeEvent(QResizeEvent *event) override ;

private:
    QString m_text;
};

class DStatusBar : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum Mode {
        Normal,
        DialogOpen,
        DialogSave
    };

    explicit DStatusBar(QWidget *parent = nullptr);

    void initUI();
    void initConnect();

    Mode mode() const;
    void setMode(Mode mode);
    void setComBoxItems(const QStringList &filters);

    QSlider *scalingSlider() const;
    QPushButton *acceptButton() const;
    QPushButton *rejectButton() const;
    QLineEdit *lineEdit() const;
    QComboBox *comboBox() const;

    QSize sizeHint() const override;

    qint64 computerSize(const DUrlList &urllist);
    int computerFolderContains(const DUrlList &urllist);
    // 计算目录和文件的个数及大小
    QVariantList calcFolderAndFile(const DUrlList &urllist);

signals:
    void modeChanged();

public slots:
    void itemSelected(const DFMEvent &event, int number);
    void updateStatusMessage();
    void handdleComputerFileSizeFinished();
    void handdleComputerFolderContainsFinished();
    // 更新目录和文件的个数及大小，并显示于状态栏
    void handleCalcFolderAndFileFinished();
    void itemCounted(const DFMEvent &event, int number);
    void setLoadingIncatorVisible(bool visible, const QString &tipText = QString());

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void clearLayoutAndAnchors();
    void initJobConnection();
    /**
     * @brief mtp挂载时, 更新底部统计信息显示
     * @param DFMEvent int
     * @return void
     */
    void showMtpStatus(const DFMEvent &event, int number);

    QString m_OnlyOneItemCounted;
    QString m_counted;
    QString m_OnlyOneItemSelected;
    QString m_selected;

    QString m_selectFolders;
    QString m_selectOnlyOneFolder;
    QString m_selectFiles;
    QString m_selectOnlyOneFile;
    QString m_selectedNetworkOnlyOneFolder;

    int m_fileCount = 0;
    qint64 m_fileSize = 0;
    int m_folderCount = 0;
    int m_folderContains = 0;

    QHBoxLayout *m_layout;
    DFMElidLabel *m_label = Q_NULLPTR;
    DPictureSequenceView *m_loadingIndicator;
    bool m_loadingIndicatorInited = false;

    QSlider *m_scaleSlider;

    QPushButton *m_acceptButton = Q_NULLPTR;
    QPushButton *m_rejectButton = Q_NULLPTR;
    QLineEdit *m_lineEdit = Q_NULLPTR;
    QComboBox *m_comboBox = Q_NULLPTR;
    QLabel *m_lineEditLabel = Q_NULLPTR;
    QLabel *m_comboBoxLabel = Q_NULLPTR;
    DFileStatisticsJob *m_fileStatisticsJob = nullptr;
    bool m_isjobDisconnect = true;

    Mode m_mode = Normal;
};

#endif // DSTATUSBAR_H
