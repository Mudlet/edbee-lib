// edbee - Copyright (c) 2012-2025 by Rick Blommers and contributors
// SPDX-License-Identifier: MIT

#include "texteditorautocompletecomponenttest.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "edbee/edbee.h"
#include "edbee/models/textautocompleteprovider.h"
#include "edbee/models/textdocument.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/components/texteditorautocompletecomponent.h"
#include "edbee/views/components/texteditorcomponent.h"

namespace edbee {

namespace {

class AutoCompleteFixture
{
public:
    AutoCompleteFixture()
        : widget()
        , provider(new StringTextAutoCompleteProvider())
        , editor(widget.textEditorComponent())
        , autocomplete(widget.autoCompleteComponent())
        , list(autocomplete->listWidget())
    {
        widget.resize(640, 320);
        widget.show();

        provider->add("compare");
        provider->add("complete");
        provider->add("compose");
        widget.textDocument()->autoCompleteProviderList()->giveProvider(provider);

        QApplication::processEvents();
        editor->setFocus();
        QApplication::processEvents();
    }

    void typePrefix(const QString& prefix)
    {
        for (const QChar ch : prefix) {
            sendEditorKey(ch.toLatin1(), QString(ch));
        }
    }

    void sendEditorKey(int key, const QString& text = QString())
    {
        QKeyEvent keyEvent(QEvent::KeyPress, key, Qt::NoModifier, text);
        QApplication::sendEvent(editor, &keyEvent);
        QApplication::processEvents();
    }

    void sendFocusedWidgetKey(int key, const QString& text = QString())
    {
        QWidget* target = QApplication::focusWidget();
        Q_ASSERT(target != nullptr);

        QKeyEvent keyEvent(QEvent::KeyPress, key, Qt::NoModifier, text);
        QApplication::sendEvent(target, &keyEvent);
        QApplication::processEvents();
    }

    void clearDocument()
    {
        while (!widget.textDocument()->text().isEmpty()) {
            sendEditorKey(Qt::Key_Backspace);
        }
    }

    TextEditorWidget widget;
    StringTextAutoCompleteProvider* provider;
    TextEditorComponent* editor;
    TextEditorAutoCompleteComponent* autocomplete;
    QListWidget* list;
};

} // namespace

void TextEditorAutoCompleteComponentTest::openingAutocompleteKeepsEditorFocused()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    testEqual(fixture.widget.textDocument()->text(), "com");
    testEqual(fixture.list->count(), 3);
    testTrue(fixture.list->isVisible());
    testEqual(fixture.editor->focusPolicy(), Qt::WheelFocus);
    testEqual(fixture.list->focusPolicy(), Qt::NoFocus);
    testTrue(fixture.list->testAttribute(Qt::WA_ShowWithoutActivating));
    testTrue(fixture.editor->hasFocus());
    testFalse(fixture.list->hasFocus());
}


void TextEditorAutoCompleteComponentTest::typingContinuesThroughEditorWhenAutocompleteIsVisible()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    fixture.sendEditorKey(Qt::Key_P, "p");

    testEqual(fixture.widget.textDocument()->text(), "comp");
    testTrue(fixture.editor->hasFocus());
    testFalse(fixture.list->hasFocus());
}


void TextEditorAutoCompleteComponentTest::typingStillRoutesThroughEditorWhenListGetsFocus()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    fixture.list->setFocusPolicy(Qt::StrongFocus);
    fixture.list->setFocus();
    QApplication::processEvents();

    testTrue(fixture.list->hasFocus());
    testTrue(QApplication::focusWidget() == fixture.list);

    fixture.sendFocusedWidgetKey(Qt::Key_P, "p");

    testEqual(fixture.widget.textDocument()->text(), "comp");
    testTrue(fixture.list->hasFocus());
}


void TextEditorAutoCompleteComponentTest::navigationKeysMoveSelectionWithoutListFocus()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    testEqual(fixture.list->currentRow(), 0);

    fixture.sendEditorKey(Qt::Key_Down);

    testEqual(fixture.list->currentRow(), 1);
    testEqual(fixture.widget.textDocument()->text(), "com");
    testTrue(fixture.editor->hasFocus());
    testFalse(fixture.list->hasFocus());
}


void TextEditorAutoCompleteComponentTest::escapeCancelsAutocompleteUntilWordIsCleared()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    fixture.sendEditorKey(Qt::Key_Escape);

    testFalse(fixture.list->isVisible());
    testTrue(fixture.editor->hasFocus());

    fixture.sendEditorKey(Qt::Key_P, "p");
    testEqual(fixture.widget.textDocument()->text(), "comp");
    testFalse(fixture.list->isVisible());

    fixture.clearDocument();
    testEqual(fixture.widget.textDocument()->text(), "");

    fixture.typePrefix("com");
    testEqual(fixture.widget.textDocument()->text(), "com");
    testTrue(fixture.list->isVisible());
}


void TextEditorAutoCompleteComponentTest::hidingAutocompleteDoesNotStealFocusFromSiblingWidget()
{
    QWidget container;
    QVBoxLayout layout(&container);
    TextEditorWidget* widget = new TextEditorWidget(&container);
    QLineEdit* sibling = new QLineEdit(&container);

    layout.addWidget(widget);
    layout.addWidget(sibling);
    container.resize(640, 360);
    container.show();
    QApplication::processEvents();

    StringTextAutoCompleteProvider* provider = new StringTextAutoCompleteProvider();
    provider->add("compare");
    widget->textDocument()->autoCompleteProviderList()->giveProvider(provider);

    TextEditorComponent* editor = widget->textEditorComponent();
    QListWidget* list = widget->autoCompleteComponent()->listWidget();

    editor->setFocus();
    QApplication::processEvents();

    for (const QChar ch : QStringLiteral("com")) {
        QKeyEvent keyEvent(QEvent::KeyPress, ch.toLatin1(), Qt::NoModifier, QString(ch));
        QApplication::sendEvent(editor, &keyEvent);
    }
    QApplication::processEvents();

    testTrue(list->isVisible());
    testTrue(editor->hasFocus());

    sibling->setFocus();
    QApplication::processEvents();
    testTrue(sibling->hasFocus());
    testTrue(QApplication::focusWidget() == sibling);

    QKeyEvent escapeKey(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QWidget* focusWidget = QApplication::focusWidget();
    Q_ASSERT(focusWidget != nullptr);
    QApplication::sendEvent(focusWidget, &escapeKey);
    QApplication::processEvents();

    testTrue(sibling->hasFocus());
    testFalse(editor->hasFocus());
    testFalse(list->hasFocus());
    testFalse(list->isVisible());
}

} // namespace edbee
