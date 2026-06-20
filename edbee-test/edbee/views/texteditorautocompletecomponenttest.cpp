// edbee - Copyright (c) 2012-2025 by Rick Blommers and contributors
// SPDX-License-Identifier: MIT

#include "texteditorautocompletecomponenttest.h"

#include "edbee/models/textautocompleteprovider.h"
#include "edbee/models/textdocument.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/components/texteditorautocompletecomponent.h"
#include "edbee/views/components/texteditorcomponent.h"
#include "edbee/views/textselection.h"

#include <QApplication>
#include <QKeyEvent>
#include <QListWidget>

namespace edbee {

void TextEditorAutoCompleteComponentTest::keepsEditorFocusWhenPopupIsShown()
{
    TextEditorWidget widget;
    widget.resize(400, 200);

    auto* provider = new StringTextAutoCompleteProvider();
    provider->add("print", TextAutoCompleteKind::Function);
    provider->add("printf", TextAutoCompleteKind::Function);
    widget.textDocument()->autoCompleteProviderList()->giveProvider(provider);

    widget.textDocument()->setText("pri");
    widget.textSelection()->setRange(3, 3);
    widget.show();
    widget.textEditorComponent()->setFocus();
    QApplication::processEvents();

    widget.autoCompleteComponent()->updateList();
    QApplication::processEvents();

    testTrue(widget.textEditorComponent()->hasFocus(), "autocomplete must not take focus from the editor");
    testFalse(widget.autoCompleteComponent()->listWidget()->hasFocus());

    QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QApplication::sendEvent(widget.textEditorComponent(), &downEvent);
    QKeyEvent enterEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(widget.textEditorComponent(), &enterEvent);
    QApplication::processEvents();

    testEqual(widget.textDocument()->text(), "printf");
    testTrue(widget.textEditorComponent()->hasFocus(), "accepting autocomplete should keep editor focus");
}

} // edbee
