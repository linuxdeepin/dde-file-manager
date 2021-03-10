/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#ifndef DFILEDIALOG_H
#define DFILEDIALOG_H

#include "dfilemanagerwindow.h"
#include "dfileview.h"
#include "dfmabstracteventhandler.h"

#include <QFileDialog>

DFM_USE_NAMESPACE

class DFileDialogPrivate;
class FileDialogStatusBar;
class DFileDialog : public DFileManagerWindow
{
    Q_OBJECT

public:
    enum CustomWidgetType {
        LineEditType = 0,
        ComboBoxType = 1
    };

    explicit DFileDialog(QWidget *parent = nullptr);
    ~DFileDialog() override;

    void setDirectory(const QString &directory);
    void setDirectory(const QDir &directory);
    QDir directory() const;

    void setDirectoryUrl(const DUrl &directory);
    QUrl directoryUrl() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void selectUrl(const QUrl &url);
    QList<QUrl> selectedUrls() const;

    void addDisableUrlScheme(const QString &scheme);

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString &filter);
    QString modelCurrentNameFilter() const;
    QString selectedNameFilter() const;
    void selectNameFilterByIndex(int index);
    int selectedNameFilterIndex() const;

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filters);

    void setViewMode(DFileView::ViewMode mode);
    DFileView::ViewMode viewMode() const;

    void setFileMode(QFileDialog::FileMode mode);

    void setAllowMixedSelection(bool on);

    void setAcceptMode(QFileDialog::AcceptMode mode);
    QFileDialog::AcceptMode acceptMode() const;

    void setLabelText(QFileDialog::DialogLabel label, const QString &text);
    QString labelText(QFileDialog::DialogLabel label) const;

    void setOptions(QFileDialog::Options options);
    void setOption(QFileDialog::Option option, bool on = true);
    bool testOption(QFileDialog::Option option) const;
    QFileDialog::Options options() const;

    void setCurrentInputName(const QString &name);
    void addCustomWidget(CustomWidgetType type, const QString &data);
    void beginAddCustomWidget();
    void endAddCustomWidget();
    QVariant getCustomWidgetValue(CustomWidgetType type, const QString &text) const;
    QVariantMap allCustomWidgetsValue(CustomWidgetType type) const;

    void setHideOnAccept(bool enable);
    bool hideOnAccept() const;

    DFileView *getFileView() const;

Q_SIGNALS:
    void finished(int result);
    void accepted();
    void rejected();
    void selectionFilesChanged();
    void currentUrlChanged();
    void selectedNameFilterChanged();

public Q_SLOTS:
    void accept();
    void done(int r);
    int exec();
    void open();
    void reject();

    /**
     * @brief  disableOpenBtn
     * 设置打开按钮的是否可用   bug 63430
     */
    void disableOpenBtn();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void adjustPosition(QWidget *w);

    bool fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target = 0, QVariant *resultData = 0) override;

private:
    void handleNewView(DFMBaseView *view) override;
    FileDialogStatusBar *statusBar() const;

    void onAcceptButtonClicked();
    void onRejectButtonClicked();
    void onCurrentInputNameChanged();
    void handleEnterPressed();
    void updateAcceptButtonState();

    QScopedPointer<DFileDialogPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileDialog)

    bool m_acceptCanOpenOnSave;
};

#endif // DFILEDIALOG_H
