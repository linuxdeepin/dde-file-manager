// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGEDITOR_H
#define TAGEDITOR_H

#include "dfmplugin_tag_global.h"

#include <DCrumbEdit>
#include <DArrowRectangle>

#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFocusEvent>
#include <QTimer>

namespace dfmplugin_tag {

class TagEditor final : public DTK_WIDGET_NAMESPACE::DArrowRectangle
{
    Q_OBJECT
public:
    explicit TagEditor(QWidget *const parent = nullptr, bool inTagDir = false);
    virtual ~TagEditor() override = default;
    TagEditor(const TagEditor &other) = delete;
    TagEditor &operator=(const TagEditor &other) = delete;

    void setFocusOutSelfClosing(bool value) noexcept;
    void setFilesForTagging(const QList<QUrl> &files);
    void setDefaultCrumbs(const QStringList &list);

public slots:
    void onFocusOut();
    void filterInput();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initializeWidgets();
    void initializeParameters();
    void initializeLayout();
    void initializeConnect();
    void setupEditHeight();

    void processTags();
    void updateCrumbsColor(const QMap<QString, QColor> &tagsColor);

    void onPressESC() noexcept;

    DTK_WIDGET_NAMESPACE::DCrumbEdit *crumbEdit = nullptr;
    QLabel *promptLabel = nullptr;
    QVBoxLayout *totalLayout = nullptr;
    QFrame *backgroundFrame = nullptr;

    QList<QUrl> files {};

    std::atomic<bool> flagForShown { false };

    bool isSettingDefault = false;
    bool isShowInTagDir { false };
};

}

#endif   // TAGEDITOR_H
