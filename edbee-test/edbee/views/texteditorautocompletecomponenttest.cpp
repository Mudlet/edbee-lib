// edbee - Copyright (c) 2012-2025 by Rick Blommers and contributors
// SPDX-License-Identifier: MIT

#include "texteditorautocompletecomponenttest.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QWidget>

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

        processEvents();
    }

    void typePrefix(const QString& prefix)
    {
        editor->setFocus();
        widget.controller()->replaceSelection(prefix);
        autocomplete->updateList();
        processEvents();
    }

    void sendEditorKey(int key, const QString& text = QString())
    {
        QKeyEvent keyEvent(QEvent::KeyPress, key, Qt::NoModifier, text);
        QApplication::sendEvent(editor, &keyEvent);
        processEvents();
    }

    void sendListKey(int key, const QString& text = QString())
    {
        QKeyEvent keyEvent(QEvent::KeyPress, key, Qt::NoModifier, text);
        QApplication::sendEvent(list, &keyEvent);
        processEvents();
    }

    void processEvents()
    {
        QApplication::processEvents();
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


void TextEditorAutoCompleteComponentTest::listKeyEventsForwardTextBackToEditor()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    fixture.sendListKey(Qt::Key_P, "p");

    testEqual(fixture.widget.textDocument()->text(), "comp");
    testTrue(fixture.editor->hasFocus());
    testFalse(fixture.list->hasFocus());
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


void TextEditorAutoCompleteComponentTest::enterAcceptsSuggestionFromEditorFocus()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    fixture.sendEditorKey(Qt::Key_Down);
    const QString selectedText = fixture.list->currentItem()->text();
    fixture.sendEditorKey(Qt::Key_Return);

    testEqual(fixture.widget.textDocument()->text(), selectedText);
    testTrue(fixture.editor->hasFocus());
    testFalse(fixture.list->hasFocus());
}


void TextEditorAutoCompleteComponentTest::fullWordEnterFallsThroughToEditor()
{
    AutoCompleteFixture fixture;
    fixture.provider->add("compareTo");
    fixture.typePrefix("compare");

    testTrue(fixture.list->isVisible());
    testEqual(fixture.list->currentItem()->text(), "compare");

    fixture.sendEditorKey(Qt::Key_Return);

    testEqual(fixture.widget.textDocument()->text(), "compare\n");
    testTrue(fixture.editor->hasFocus());
    testFalse(fixture.list->hasFocus());
}


void TextEditorAutoCompleteComponentTest::escapeCancelsAutocompleteUntilWordIsCleared()
{
    AutoCompleteFixture fixture;
    fixture.typePrefix("com");

    fixture.sendEditorKey(Qt::Key_Escape);
    fixture.autocomplete->updateList();
    fixture.processEvents();

    testFalse(fixture.list->isVisible());
    testTrue(fixture.editor->hasFocus());

    fixture.sendEditorKey(Qt::Key_P, "p");
    fixture.autocomplete->updateList();
    fixture.processEvents();

    testEqual(fixture.widget.textDocument()->text(), "comp");
    testFalse(fixture.list->isVisible());

    fixture.sendEditorKey(Qt::Key_Backspace);
    fixture.sendEditorKey(Qt::Key_Backspace);
    fixture.sendEditorKey(Qt::Key_Backspace);
    fixture.sendEditorKey(Qt::Key_Backspace);
    fixture.autocomplete->updateList();
    fixture.processEvents();

    testEqual(fixture.widget.textDocument()->text(), "");

    fixture.sendEditorKey(Qt::Key_C, "c");
    fixture.sendEditorKey(Qt::Key_O, "o");
    fixture.sendEditorKey(Qt::Key_M, "m");

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
    widget->controller()->replaceSelection("com");
    widget->autoCompleteComponent()->updateList();
    QApplication::processEvents();

    testTrue(editor->hasFocus());
    testTrue(list->isVisible());

    sibling->setFocus();
    QKeyEvent escapeKey(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(list, &escapeKey);
    QApplication::processEvents();

    testTrue(sibling->hasFocus());
    testFalse(editor->hasFocus());
    testFalse(list->hasFocus());
}

} // edbee
