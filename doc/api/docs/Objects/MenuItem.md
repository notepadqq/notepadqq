# MenuItem

A menu item.

[TOC]

## item.setShortcut(shortcut)

Sets the shortcut keys for this menu item.

For example:

    var menu = window.addExtensionMenuItem(api.extensionId, "My menu");
    menu.setShortcut("Ctrl+Alt+E");

## Event: 'triggered'

`function (checked) { }`

This event is emitted when the user clicks the menu item.

For example:

    var menu = window.addExtensionMenuItem(api.extensionId, "My menu");
    menu.on("triggered", function(checked) {
        console.log("Clicked!");
    });