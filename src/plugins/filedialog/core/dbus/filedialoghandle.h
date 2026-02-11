// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEDIALOGHANDLE_H
#define FILEDIALOGHANDLE_H

#include "views/filedialog.h"

#include <QFileDialog>
#include <QScopedPointer>
#include <QDBusVariant>

class FileDialogHandlePrivate;
class FileDialogHandle : public QObject
{
    Q_OBJECT

public:
    explicit FileDialogHandle(QWidget *parent = nullptr);
    ~FileDialogHandle();

public Q_SLOTS:
    void setParent(QWidget *parent);
    QWidget *widget() const;

    void setDirectory(const QString &directory);
    void setDirectory(const QDir &directory);
    QDir directory() const;

    void setDirectoryUrl(const QUrl &directory);
    QUrl directoryUrl() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void selectUrl(const QUrl &url);
    QList<QUrl> selectedUrls() const;

    void addDisableUrlScheme(const QString &scheme);

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString &filter);
    QString selectedNameFilter() const;
    void selectNameFilterByIndex(int index);
    int selectedNameFilterIndex() const;
    qulonglong winId() const;

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filters);

    void setViewMode(QFileDialog::ViewMode mode);
    QFileDialog::ViewMode viewMode() const;

    void setFileMode(QFileDialog::FileMode mode);

    void setAcceptMode(QFileDialog::AcceptMode mode);
    QFileDialog::AcceptMode acceptMode() const;

    void setLabelText(QFileDialog::DialogLabel label, const QString &text);
    QString labelText(QFileDialog::DialogLabel label) const;

    void setOptions(QFileDialog::Options options);
    void setOption(QFileDialog::Option option, bool on = true);
    QFileDialog::Options options() const;
    bool testOption(QFileDialog::Option option) const;

    void setCurrentInputName(const QString &name);
    void addCustomWidget(int type, const QString &data);
    QDBusVariant getCustomWidgetValue(int type, const QString &text) const;
    QVariantMap allCustomWidgetsValue(int type) const;
    void beginAddCustomWidget();
    void endAddCustomWidget();

    void setAllowMixedSelection(bool on);

    void setHideOnAccept(bool enable);
    bool hideOnAccept() const;

    void show();
    void hide();
    void accept();
    void done(int r);
    int exec();
    void open();
    void reject();

private:
    void addDefaultSettingForWindow(QPointer<filedialog_core::FileDialog> dialog);
    void setWindowStayOnTop();

Q_SIGNALS:
    void finished(int result);
    void accepted();
    void rejected();
    void selectionFilesChanged();
    void currentUrlChanged();
    void selectedNameFilterChanged();

private:
    QScopedPointer<FileDialogHandlePrivate> d_ptr;
    bool isSetAcceptMode { false };
    bool isSetNameFilters { false };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), FileDialogHandle)
    Q_DISABLE_COPY(FileDialogHandle)
};

#endif   // FILEDIALOGHANDLE_H
