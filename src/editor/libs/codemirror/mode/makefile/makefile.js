// CodeMirror, copyright (c) by Marijn Haverbeke and others
// Distributed under an MIT license: http://codemirror.net/LICENSE

(function(mod) {
  if (typeof exports == "object" && typeof module == "object") // CommonJS
    mod(require("../../lib/codemirror"));
  else if (typeof define == "function" && define.amd) // AMD
    define(["../../lib/codemirror"], mod);
  else // Plain browser env
    mod(CodeMirror);
})(function(CodeMirror) {
"use strict";

CodeMirror.defineMode('makefile', function() {

  var words = {};
  function define(style, string) {
    var split = string.split(' ');
    for(var i = 0; i < split.length; i++) {
      words[split[i]] = style;
    }
  };

  function tokenBase(stream, state) {
    if (stream.eatSpace()) return null;

    var sol = stream.sol();
    var ch = stream.next();

    if (ch === '@') { return "atom"; }
    if (ch === '#') {
      if (sol && stream.eat('!')) {
        stream.skipToEnd();
        return 'meta'; // 'comment'?
      }
      stream.skipToEnd();
      return 'comment';
    }
    if (ch === '$' && stream.eat('(')) {
      state.tokens.unshift(tokenDollar);
      return tokenize(stream, state);
    }
    if (ch === '$' && stream.eat('$') && stream.match(/^[\w]+/)) { return "quote"; }
    if (ch === '$' && stream.eat('@')) { return "quote"; }
    if (ch === '$' && stream.eat('<')) { return "quote"; }
    if (ch === '$' && stream.eat('^')) { return "quote"; }

    if (ch === 'i' && stream.match('feq ')) {
      stream.skipToEnd();
      return "meta";
    }
    if (ch === 'e' && stream.match('lse')) { return "meta"; }
    if (ch === 'e' && stream.match('ndif')) { return "meta"; }
    if (ch === 'i' && stream.match('nclude ')) { return "string"; }

    if (stream.match(/^[\w.]+:/)) { return "def"; }

    stream.eatWhile(/[\w-]/);
    var cur = stream.current();
    //if (stream.peek() === '=' && /\w+/.test(cur)) return 'def';
    return words.hasOwnProperty(cur) ? words[cur] : null;
  }

  function tokenString(quote) {
    return function(stream, state) {
      var next, end = false, escaped = false;
      while ((next = stream.next()) != null) {
        if (next === quote && !escaped) {
          end = true;
          break;
        }
      }
      if (end || !escaped) {
        state.tokens.shift();
      }
      return (quote === ')' ? 'quote' : 'string');
    };
  };


  var tokenDollar = function(stream, state) {
    if (state.tokens.length > 1) stream.eat('$');
    var ch = stream.next(), hungry = /\w/;
    state.tokens[0] = tokenString(')');
    return tokenize(stream, state);
  };

  function tokenize(stream, state) {
    return (state.tokens[0] || tokenBase) (stream, state);
  };

  return {
    startState: function() {return {tokens:[]};},
    token: function(stream, state) {
      return tokenize(stream, state);
    }
  };
});

CodeMirror.defineMIME('text/x-makefile', 'makefile');

});
