// edbee - Copyright (c) 2012-2025 by Rick Blommers and contributors
// SPDX-License-Identifier: MIT

#include "texteditorautocompletecomponenttest.h"

#include <QApplication>
#include <QKeyEvent>
#include <QListWidget>

#include "edbee/models/textautocompleteprovider.h"
#include "edbee/models/textdocument.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/components/texteditorautocompletecomponent.h"
#include "edbee/views/components/texteditorcomponent.h"

namespace edbee {

void TextEditorAutoCompleteComponentTest::typingKeepsEditorFocusedWhenAutocompleteIsVisible()
{
    TextEditorWidget widget;
    widget.resize(640, 320);
    widget.show();
    QApplication::processEvents();

    StringTextAutoCompleteProvider* provider = new StringTextAutoCompleteProvider();
    provider->add("compare");
    widget.textDocument()->autoCompleteProviderList()->giveProvider(provider);

    TextEditorComponent* editor = widget.textEditorComponent();
    QListWidget* list = widget.autoCompleteComponent()->listWidget();

    editor->setFocus();
    widget.controller()->replaceSelection("com");
    widget.autoCompleteComponent()->updateList();
    QApplication::processEvents();

    testEqual(widget.textDocument()->text(), "com");
    testTrue(editor->hasFocus());
    testFalse(list->hasFocus());
    testEqual(list->count(), 1);

    QKeyEvent textKey(QEvent::KeyPress, Qt::Key_P, Qt::NoModifier, "p");
    QApplication::sendEvent(editor, &textKey);
    QApplication::processEvents();

    testEqual(widget.textDocument()->text(), "comp");
    testTrue(editor->hasFocus());
    testFalse(list->hasFocus());
}

} // edbee
