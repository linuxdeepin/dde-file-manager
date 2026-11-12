// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tageditor.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"

#include <dfm-base/utils/windowutils.h>

#include <DGuiApplicationHelper>

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QShowEvent>
#include <QCloseEvent>
#include <QWindow>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_tag;

TagEditor::TagEditor(QWidget *const parent, bool inTagDir)
    : DArrowRectangle { DArrowRectangle::ArrowTop, parent }, isShowInTagDir(inTagDir)
{
    initializeWidgets();
    initializeParameters();
    initializeLayout();
    initializeConnect();

    installEventFilter(this);

    // NOTE: in wayland you must specify the window flags manually
    if (dfmbase::WindowUtils::isWayLand())
        // Qt::Popup turns this into a grabbed xdg_popup on wayland: the compositor
        // routes keyboard input to it and dismisses it on outside clicks, while
        // Qt::ToolTip (an ungrabbed popup) gets neither
        setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Popup | Qt::FramelessWindowHint);
    else
        setWindowFlags(Qt::ToolTip);
}

void TagEditor::setFocusOutSelfClosing(bool value) noexcept
{
    bool excepted { !value };
    flagForShown.compare_exchange_strong(excepted, value, std::memory_order_release);
}

void TagEditor::setFilesForTagging(const QList<QUrl> &files)
{
    this->files = files;
}

void TagEditor::setDefaultCrumbs(const QStringList &list)
{
    const auto &tagsMap = TagManager::instance()->getTagsColor(list);
    updateCrumbsColor(tagsMap);
}

void TagEditor::onFocusOut()
{
    if (flagForShown.load(std::memory_order_acquire)) {
        QString text = crumbEdit->toPlainText().remove(QChar::ObjectReplacementCharacter).trimmed();
        if (!text.isEmpty())
            crumbEdit->appendCrumb(text);
        processTags();
        close();
    }
}

void TagEditor::filterInput()
{
    TagHelper::instance()->crumbEditInputFilter(crumbEdit);
}

void TagEditor::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape: {
        this->processTags();
        event->accept();
        this->close();
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        QObject::disconnect(this, &TagEditor::windowDeactivate, this, &TagEditor::onFocusOut);
        processTags();
        event->accept();
        close();
        break;
    }
    default:
        break;
    }

    DArrowRectangle::keyPressEvent(event);
}

void TagEditor::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void TagEditor::mousePressEvent(QMouseEvent *event)
{
    // In popup mode QWidgetWindow re-routes presses outside the popup to the
    // active popup widget itself. On wayland the compositor neither dismisses
    // an xdg popup nor deactivates it when the click lands on another surface
    // of the same client, so dismiss the editor ourselves when the press is
    // outside its rect — the same way QMenu closes (closeEvent commits input).
    if (!rect().contains(event->pos())) {
        close();
        return;
    }

    DArrowRectangle::mousePressEvent(event);
}

void TagEditor::showEvent(QShowEvent *event)
{
    DArrowRectangle::showEvent(event);
    // make sure the caret is ready for typing as soon as the editor pops up
    crumbEdit->setFocus();
    // close on any press outside the editor window while it is open, no matter
    // which surface of the app receives it (popup mode routing is unreliable here)
    qApp->installEventFilter(this);
}

bool TagEditor::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        const QWindow *targetWin = nullptr;
        if (auto *win = qobject_cast<QWindow *>(watched))
            targetWin = win;
        else if (auto *widget = qobject_cast<QWidget *>(watched))
            targetWin = widget->window()->windowHandle();

        if (targetWin && targetWin != windowHandle()) {
            // ignore presses that land inside the editor's visible geometry even if
            // hit-testing routes them to another window (defensive, e.g. on
            // compositors where an anchored popup is not hit-testable)
            const auto *me = static_cast<QMouseEvent *>(event);
            const QRect editorGeom(mapToGlobal(QPoint(0, 0)), size());
            if (!editorGeom.contains(me->globalPosition().toPoint())) {
                close();
                return true;
            }
        }
    }

    return DArrowRectangle::eventFilter(watched, event);
}

void TagEditor::closeEvent(QCloseEvent *event)
{
    qApp->removeEventFilter(this);
    // on wayland the compositor may dismiss the xdg popup directly (outside click),
    // which closes the window without windowDeactivate/onFocusOut, so commit the
    // pending input here to avoid losing what was typed (no-op if already committed)
    if (dfmbase::WindowUtils::isWayLand() && flagForShown.load(std::memory_order_acquire)) {
        QString text = crumbEdit->toPlainText().remove(QChar::ObjectReplacementCharacter).trimmed();
        if (!text.isEmpty())
            crumbEdit->appendCrumb(text);
        processTags();
    }
    DArrowRectangle::closeEvent(event);
}

void TagEditor::initializeWidgets()
{
    crumbEdit = new DCrumbEdit;
    crumbEdit->setObjectName("CrumbEdit");
    crumbEdit->setAccessibleName("CrumbEdit");
    promptLabel = new QLabel(tr("Input tag info, such as work, family. A comma is used between two tags."));
    totalLayout = new QVBoxLayout;
    totalLayout->setSizeConstraint(QLayout::SetFixedSize);
    backgroundFrame = new QFrame;
}

void TagEditor::initializeParameters()
{
    crumbEdit->setFixedSize(140, 40);
    crumbEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbEdit->setCrumbReadOnly(true);
    crumbEdit->setCrumbRadius(2);
    setupEditHeight();

    promptLabel->setFixedWidth(140);
    promptLabel->setWordWrap(true);
    backgroundFrame->setContentsMargins(QMargins { 0, 0, 0, 0 });

    setMargin(0);
    setFixedWidth(140);
    setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    updateBackgroundColor();
}

void TagEditor::initializeLayout()
{
    totalLayout->addStretch(1);
    totalLayout->addWidget(crumbEdit);
    totalLayout->addSpacing(8);
    totalLayout->addWidget(promptLabel);
    totalLayout->addStretch(1);

    backgroundFrame->setLayout(totalLayout);
    setContent(backgroundFrame);
}

void TagEditor::initializeConnect()
{
    QObject::connect(this, &TagEditor::windowDeactivate, this, &TagEditor::onFocusOut);
    QObject::connect(crumbEdit, &QTextEdit::textChanged, this, &TagEditor::filterInput);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TagEditor::updateBackgroundColor);

    if (!isShowInTagDir) {
        QObject::connect(crumbEdit, &DCrumbEdit::crumbListChanged, this, [=] {
            if (!crumbEdit->property("updateCrumbsColor").toBool())
                processTags();
        });
    }
}

void TagEditor::setupEditHeight()
{
    QTextEdit *edit = qobject_cast<QTextEdit *>(crumbEdit);
    if (edit) {
        auto doc = edit->document();
        doc->setDocumentMargin(doc->documentMargin() + 5);
        auto layout = edit->document()->documentLayout();
        connect(layout, &QAbstractTextDocumentLayout::documentSizeChanged,
                this, [this, edit] {
                    qreal docHeight = edit->document()->size().height();
                    QMargins margins = edit->contentsMargins();
                    int totalHeight = qCeil(docHeight) + margins.top() + margins.bottom();
                    edit->setFixedHeight(totalHeight);
                    QTimer::singleShot(0, this, &TagEditor::resizeWithContent);
                });
    }
}

void TagEditor::processTags()
{
    QList<QString> tags;
    for (const QString &tag : crumbEdit->crumbList()) {
        QString trimmed = tag.trimmed();
        if (!trimmed.isEmpty())
            tags << trimmed;
    }

    QList<QUrl> tempFiles = files;

    updateCrumbsColor(TagManager::instance()->assignColorToTags(tags));
    TagManager::instance()->setTagsForFiles(tags, tempFiles);
}

void TagEditor::updateCrumbsColor(const QMap<QString, QColor> &tagsColor)
{
    if (tagsColor.isEmpty())
        return;

    crumbEdit->setProperty("updateCrumbsColor", true);
    crumbEdit->clear();

    for (auto it = tagsColor.begin(); it != tagsColor.end(); ++it) {
        DCrumbTextFormat format = crumbEdit->makeTextFormat();
        format.setText(it.key());

        format.setBackground(QBrush(it.value()));
        format.setBackgroundRadius(5);

        crumbEdit->insertCrumb(format);
    }

    crumbEdit->setProperty("updateCrumbsColor", false);
}

void TagEditor::updateBackgroundColor()
{
    setBorderColor(palette().color(QPalette::Base));
    setBackgroundColor(palette().color(QPalette::Base));
}
