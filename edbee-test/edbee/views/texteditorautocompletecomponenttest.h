// edbee - Copyright (c) 2012-2025 by Rick Blommers and contributors
// SPDX-License-Identifier: MIT

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextEditorAutoCompleteComponentTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void typingKeepsEditorFocusedWhenAutocompleteIsVisible();
};

DECLARE_TEST(edbee::TextEditorAutoCompleteComponentTest);

} // edbee
