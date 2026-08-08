// edbee - Copyright (c) 2012-2025 by Rick Blommers and contributors
// SPDX-License-Identifier: MIT

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextEditorAutoCompleteComponentTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void openingAutocompleteKeepsEditorFocused();
    void typingContinuesThroughEditorWhenAutocompleteIsVisible();
    void listKeyEventsForwardTextBackToEditor();
    void navigationKeysMoveSelectionWithoutListFocus();
    void enterAcceptsSuggestionFromEditorFocus();
    void fullWordEnterFallsThroughToEditor();
    void escapeCancelsAutocompleteUntilWordIsCleared();
    void hidingAutocompleteDoesNotStealFocusFromSiblingWidget();
};

DECLARE_TEST(edbee::TextEditorAutoCompleteComponentTest);

} // edbee
