# Editor

Represents the content of a standard tab in Notepadqq.

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

Set line wrapping behavior for lines longer than the editor width.
Wraps if `wrap == true`, scrolls otherwise.

## editor.setSelectionsText(values)

Replace the current selections with different texts. `values` is an array of
strings.

See [`editor.selectedTexts()`](#editorselectedtexts).

## editor.setValue(val)

Replace the entire editor content with the provided value. This function might
be slow: avoid it when not necessary.

## editor.setZoomFactor(factor)

Set the zoom for this editor. A factor of `1.0` means no zoom.

Example:

    window.currentEditor().setZoomFactor(1.75)

## editor.value()

Returns the entire editor content. This function might be slow: avoid it when not
necessary.

## editor.zoomFactor()

Returns the editor zoom factor.