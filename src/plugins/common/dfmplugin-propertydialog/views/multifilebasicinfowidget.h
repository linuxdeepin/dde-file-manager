// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIFILEBASICINFOWIDGET_H
#define MULTIFILEBASICINFOWIDGET_H

#include "dfmplugin_propertydialog_global.h"
#include "skippartiallycheckbox.h"

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/filescanner.h>

#include <QPointer>
#include <QSet>

#include <DArrowLineDrawer>

namespace dfmplugin_propertydialog {

class MultiFileBasicInfoWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit MultiFileBasicInfoWidget(const QList<QUrl> &urls,
                                      QWidget *parent = Q_NULLPTR);
    virtual ~MultiFileBasicInfoWidget() override;

    void getOrgHideBoxState(FilePropertyState &states);

Q_SIGNALS:
    void hideBoxStateChanged(int state);

private Q_SLOTS:
    void updateFilesCountAndSizeLabel(const DFMBASE_NAMESPACE::FileScanner::ScanResult &result);
    void filesHideStateChanged(int state);

private:
    void initUI();
    void loadData(const QList<QUrl> &urls);
    void setFilesCountAndSize(const QList<QUrl> &urls);
    void setAccessTime(const QList<QUrl> &urls);
    void setModifyTime(const QList<QUrl> &urls);
    void setHideState(const QList<QUrl> &urls);
    DFMBASE_NAMESPACE::KeyValueLabel *createValueLabel(QFrame *frame,
                                                       const QString &leftValue);
    void calculateFileCount(const QList<QUrl> &urls,
                            int &dirCount,
                            int &fileCount);

    // Asynchronous FileScanner retirement (mirrors BasicStatusBarPrivate):
    // never wait on the worker thread on the UI thread, so closing the dialog
    // while a large multi-file scan is in progress cannot freeze the UI
    // (cf. ~FileScanner -> QThread::wait()).
    void discardCurrentScanner();
    void retireScanner(DFMBASE_NAMESPACE::FileScanner *scanner);

    DFMBASE_NAMESPACE::KeyValueLabel *filesSize { Q_NULLPTR };
    DFMBASE_NAMESPACE::KeyValueLabel *filesCount { Q_NULLPTR };
    DFMBASE_NAMESPACE::KeyValueLabel *accessTime { Q_NULLPTR };
    DFMBASE_NAMESPACE::KeyValueLabel *modifyTime { Q_NULLPTR };
    SkipPartiallyCheckBox *hideFile { Q_NULLPTR };
    QPointer<DFMBASE_NAMESPACE::FileScanner> fileCalculationUtils;
    QSet<DFMBASE_NAMESPACE::FileScanner *> retiredScanners;
    QSet<DFMBASE_NAMESPACE::FileScanner *> finishedScanners;
    quint64 scanGeneration { 0 };
    static constexpr int kMaxRetiredScanners { 2 };
};

}

#endif
