# Editor

Description

[TOC]

## editor.fileName()

Get the file name associated with this editor.

## editor.fileOnDiskChanged()

Returns `true` if the file has been changed or deleted from outside of
Notepadqq since the last time we read it.

## editor.isClean()

Returns `true` if no changes has been made since the last time the file was
saved.

## editor.language()

Returns the identifier of the current language (e.g. "plaintext", "javascript",
"csharp", ...).

## editor.markClean()

Marks the editor as if no changes were made since the last time the file was
saved.

## editor.markDirty()

Marks the editor as if some changes were made since the last time the file was
saved.

## editor.selectedTexts()

Returns an array containing the texts within the current selections (you can
have more than one selection).

## editor.setFileName(filename)

Set the file name associated with this editor.

For example:

    window.currentEditor().setFileName("/tmp/test.txt");

## editor.setFileOnDiskChanged(changed)

Behave as if the file was changed or deleted (or, if `changed == false`, the
opposite) from outside of Notepadqq since the last time we read it.

For example, if `changed == false` Notepadqq will not ask to save the file
when the editor will close.

## editor.setFocus()

Give focus to the editor, so that the user can start
typing. Note that calling won't automatically switch to
the tab where the editor is. Use `EditorTabWidget::setCurrentIndex()` [TODO]
and `TopEditorContainer::setFocus()` [TODO] for that.

## editor.setLanguage()

Set the language for the editor (e.g. "plaintext", "javascript", "csharp", ...).

See [editor.language()](#editorlanguage).

## editor.setLanguageFromFileName([fileName])

Set the language for the editor based on a file name. If not provided, the
file name associated with the editor is used (see [editor.fileName()](#editorfilename)).

## editor.setLineWrap(wrap)

Description

## editor.setSelectionsText(values)

Description

## editor.setValue(val)

Description

## editor.setZoomFactor(factor)

Description

## editor.value()

Description

## editor.zoomFactor()

Description