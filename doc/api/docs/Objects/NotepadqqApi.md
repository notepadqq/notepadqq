# NotepadqqApi

Provides access to the Notepadqq API.

[TOC]

## NotepadqqApi.connect()
## NotepadqqApi.connect(connectedCallback)
## NotepadqqApi.connect(socketPath, extensionId)
## NotepadqqApi.connect(socketPath, extensionId, connectedCallback)

Connects to Notepadqq and returns a new [`NotepadqqApi`](NotepadqqApi)
instance.

If a function is provided with `connectedCallback`, it will be called
as soon as the connection is completed. The parameters passed to the
callback are:

  - `api`: the constructed [`NotepadqqApi`](NotepadqqApi) object.
  
If `socketPath` and `extensionId` are provided, they'll be used for the
connection. Otherwise, their values are taken respectively from the first and
the second command line argument (with Node, they correspond to
`process.argv[2]` and `process.argv[3]`).

For example:

    var NotepadqqApi = require("notepadqq-api").NotepadqqApi

    NotepadqqApi.connect(function(api) {
        // Connected
    });

## api.onWindowInitialization(callback)

Launch a callback for each currently open window and for each future window.

This is preferable to the `'newWindow'` event of Notepadqq, because it could
happen that the extension isn't ready soon enough to receive the `'newWindow'`
event for the first window. This method, instead, ensures that the passed
callback will be called once and only once for each current or future window.

Example:

    var NotepadqqApi = require("notepadqq-api").NotepadqqApi

    NotepadqqApi.connect(function(api) {
        api.onWindowInitialization(function(window) {
            // Do something
        });
    });

## api.extensionId

Get the id assigned to this extension by Notepadqq.

## api.notepadqq

Get an instance of the main [`Notepadqq`](Notepadqq) object.