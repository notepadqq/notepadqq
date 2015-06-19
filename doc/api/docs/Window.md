# Window

Represents a window of Notepadqq.

[TOC]

## window.currentEditor()

Returns an instance of the currently opened [`Editor`](Editor).

## window.addExtensionMenuItem(extensionId, text)

Create a menu item within the 'Extensions' menu in Notepadqq.
The parameter `extensionId` is the id of the current extension, and `text` is
the title of the menu item.

Returns the created menu item.

Example:

	NotepadqqApi.connect(function(api) {
		api.onWindowInitialization(function(window) {
			var menu = window.addExtensionMenuItem(api.extensionId, "My menu")
		});
	});