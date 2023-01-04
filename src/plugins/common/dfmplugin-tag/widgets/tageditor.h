/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
