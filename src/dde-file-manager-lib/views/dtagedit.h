// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTAGEDIT_H
#define DTAGEDIT_H

#include <set>

#include "durl.h"
#include "dcrumbedit.h"
#include "darrowrectangle.h"


#include <QFrame>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFocusEvent>
#include <QTimer>

using namespace Dtk::Widget;


class DTagEdit final : public DArrowRectangle
{
    Q_OBJECT

public:
    explicit DTagEdit(QWidget *const parent = nullptr);
    virtual ~DTagEdit() override = default;
    DTagEdit(const DTagEdit &other) = delete;
    DTagEdit &operator=(const DTagEdit &other) = delete;

    void setFocusOutSelfClosing(bool value) noexcept;
    void setFilesForTagging(const QList<DUrl> &files);
    void setDefaultCrumbs(const QStringList &list);

public slots:
    void onFocusOut();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initializeWidgets();
    void initializeParameters();
    void initializeLayout();
    void initializeConnect();

    void processTags();
    void updateCrumbsColor(const QMap<QString, QColor> &tagsColor);
    QMap<QString, QColor> tagsColor(const QStringList &tagList);

    void onPressESC() noexcept;

    DCrumbEdit *m_crumbEdit{ nullptr };
    QLabel *m_promptLabel{ nullptr };
    QVBoxLayout *m_totalLayout{ nullptr };
    QFrame *m_BGFrame{ nullptr };

    QList<DUrl> m_files{};

    std::atomic<bool> m_flagForShown{ false };

    QTimer m_waitForMoreCrumbChanged;   //用于等待短时间内的多次CrumbChanged
};



#endif // DTAGEDIT_H
