// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMADVANCESEARCHBAR_H
#define DFMADVANCESEARCHBAR_H

#include <QVBoxLayout>

#include <QComboBox>
#include <QLabel>
#include <QWidget>
#include <dfmglobal.h>
#include <dboxwidget.h>
#include <DCommandLinkButton>

typedef struct fileFilter FileFilter;
DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMAdvanceSearchBar : public DBoxWidget
{
    Q_OBJECT
public:
    explicit DFMAdvanceSearchBar(QWidget *parent = nullptr);

    void resetForm(bool updateView = true);

    void updateFilterValue(const FileFilter *filter);

signals:
    void optionChanged(QMap<int, QVariant> formData, bool updateView);

private slots:
    void onOptionChanged();
    void onResetButtonPressed();

protected slots:
    void hideEvent(QHideEvent *) override;

private:
    void initUI();
    void initConnection();

    enum LabelIndex {
        SEARCH_RANGE, FILE_TYPE, SIZE_RANGE, DATE_RANGE, ACCESS_DATE_RANGE, CREATE_DATE_RANGE, LABEL_COUNT,
        TRIGGER_SEARCH = 114514
    };

    QBoxLayout *mainLayout;
    QLabel *asbLabels[LABEL_COUNT];
    QComboBox *asbCombos[LABEL_COUNT];
    DCommandLinkButton *resetBtn;
    bool needSearchAgain = true;
    bool allowUpdateView = true;

    // 记录下拉框选项信息，用于更新下拉框选项
    QMap<QPair<quint64, quint64>, int> sizeRangeMap;
    QMap<QVariant, int> dateRangeMap;
};

DFM_END_NAMESPACE

#endif // DFMADVANCESEARCHBAR_H
