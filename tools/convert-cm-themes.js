/**
 * Converts CodeMirror CSS theme files to Monaco Editor theme definitions.
 *
 * Usage: node tools/convert-cm-themes.js [theme names...]
 *   If no theme names given, processes all .css files in the CM theme directory.
 *
 * Example: node tools/convert-cm-themes.js monokai dracula ambiance
 */
const fs = require('fs');
const path = require('path');

const CM_THEME_DIR = path.join(__dirname, '..', 'src', 'editor', 'libs', 'codemirror', 'theme');
const OUT_DIR = path.join(__dirname, '..', 'src', 'editor', 'libs', 'monaco-addons', 'themes');

// ── CM class to Monaco token mapping ──

const TOKEN_MAP = {
  'cm-keyword':      { token: 'keyword' },
  'cm-atom':         { token: 'keyword.other' },
  'cm-number':       { token: 'number' },
  'cm-def':          { token: 'identifier' },
  'cm-variable':     { token: 'identifier' },
  'cm-variable-2':   { token: 'identifier' },
  'cm-variable-3':   { token: 'type' },
  'cm-type':         { token: 'type' },
  'cm-property':     { token: 'property' },
  'cm-operator':     { token: 'delimiter' },
  'cm-comment':      { token: 'comment' },
  'cm-string':       { token: 'string' },
  'cm-string-2':     { token: 'string' },
  'cm-meta':         { token: 'meta' },
  'cm-qualifier':    { token: 'keyword' },
  'cm-builtin':      { token: 'keyword' },
  'cm-bracket':      { token: 'delimiter.bracket' },
  'cm-tag':          { token: 'tag' },
  'cm-attribute':    { token: 'attribute.name' },
  'cm-hr':           { token: 'tag' },
  'cm-link':         { token: 'string.link' },
  'cm-special':      { token: 'keyword' },
  'cm-header':       { token: 'heading' },
  'cm-quote':        { token: 'quote' },
  'cm-error':        { token: 'invalid' },
  'cm-invalidchar':  { token: 'invalid' },
};

// ── CM chrome selector → Monaco color key ──

const CHROME_MAP = [
  // editor background/foreground (match .CodeMirror alone or .cm-s-x.CodeMirror)
  { test: /\.CodeMirror(?:\s|,|$)/,   bg: 'editor.background',      fg: 'editor.foreground' },
  // cursor
  { test: /CodeMirror-cursor/,        borderFg: 'editorCursor.foreground' },
  // selection
  { test: /CodeMirror-selected/,      bg: 'editor.selectionBackground' },
  // active line
  { test: /CodeMirror-activeline-background/, bg: 'editor.lineHighlightBackground' },
  // gutters background
  { test: /CodeMirror-gutters/,       bg: 'editorGutter.background' },
  // line number
  { test: /CodeMirror-linenumber/,    fg: 'editorLineNumber.foreground' },
  // matching bracket
  { test: /CodeMirror-matchingbracket/, fg: 'editorBracketMatch.foreground' },
];

// ── Helpers ──

function parseCssRules(css) {
  const blocks = [];
  const blockRe = /([^{]+)\{([^}]*)\}/g;
  let m;
  while ((m = blockRe.exec(css)) !== null) {
    const selector = m[1].trim();
    const body = m[2].trim();
    if (!body) continue;
    const props = {};
    const propRe = /([\w-]+)\s*:\s*([^;]+)/g;
    let p;
    while ((p = propRe.exec(body)) !== null) {
      props[p[1].toLowerCase()] = p[2].trim();
    }
    blocks.push({ selector, props });
  }
  return blocks;
}

function extractBackgroundColor(raw) {
  if (!raw) return undefined;
  // Extract just the color from a CSS background shorthand (e.g., "none repeat scroll 0% 0% rgba(...)")
  const colorMatch = raw.match(/(#[0-9a-fA-F]{3,8}|rgba?\([^)]+\)|[a-zA-Z]+)\s*$/);
  return colorMatch ? colorMatch[1] : raw;
}

function extractColors(props) {
  const out = {};
  if (props.color) out.color = cleanImportant(props.color);
  if (props.background || props['background-color']) {
    out.background = extractBackgroundColor(cleanImportant(props.background || props['background-color']));
  }
  // border-left (used for cursor)
  if (props['border-left']) {
    const cleaned = cleanImportant(props['border-left']);
    const c = cleaned.match(/(#[0-9a-fA-F]+|rgba?\([^)]+\)|[a-zA-Z]+)\s*$/);
    if (c) out.borderColor = c[1];
  }
  if (props['font-style']) out.fontStyle = props['font-style'];
  if (props['font-weight']) out.fontWeight = props['font-weight'];
  if (props['text-decoration']) out.textDecoration = props['text-decoration'];
  return out;
}

// Get all cm-* classes from a selector, excluding theme classes (cm-s-<name>)
function extractCmClasses(selector) {
  const classes = [];
  const re = /\.(cm-[\w-]+)/g;
  let m;
  while ((m = re.exec(selector)) !== null) {
    if (!m[1].startsWith('cm-s-')) {
      classes.push(m[1]);
    }
  }
  return classes;
}

// Determine if a color is "dark" based on luminance
function isDark(bgColor) {
  if (!bgColor) return true;
  // Try named colors
  let hex = null;
  if (/^[a-zA-Z]+$/.test(bgColor)) {
    const named = namedColorToHex(bgColor);
    if (named) hex = named;
  } else if (bgColor.startsWith('rgba') || bgColor.startsWith('rgb')) {
    const h = rgbaToHex(bgColor);
    if (h) hex = h.replace('#', '');
  } else {
    hex = normalizeHex(bgColor);
  }
  if (!hex) return true; // default dark
  const r = parseInt(hex.slice(0, 2), 16);
  const g = parseInt(hex.slice(2, 4), 16);
  const b = parseInt(hex.slice(4, 6), 16);
  // Relative luminance
  const lum = 0.299 * r + 0.587 * g + 0.114 * b;
  return lum < 128;
}

function cleanImportant(c) {
  return c ? c.replace(/\s*!important\s*/g, '').trim() : c;
}

function normalizeHex(color) {
  if (!color) return null;
  color = cleanImportant(color);
  // rgba(255,255,255,0.1) → try to extract, return as-is for Monaco
  if (color.startsWith('rgba') || color.startsWith('rgb')) {
    const m = color.match(/rgba?\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)/);
    if (m) {
      const r = parseInt(m[1]).toString(16).padStart(2, '0');
      const g = parseInt(m[2]).toString(16).padStart(2, '0');
      const b = parseInt(m[3]).toString(16).padStart(2, '0');
      return r + g + b;
    }
    return null;
  }
  // named color like "white", "transparent"
  if (/^[a-zA-Z]+$/.test(color)) {
    return null; // keep as-is
  }
  // hex
  let h = color.replace('#', '');
  if (h.length === 3) {
    h = h[0] + h[0] + h[1] + h[1] + h[2] + h[2];
  }
  if (h.length === 6) return h;
  return null;
}

function namedColorToHex(name) {
  const map = {
    'black': '000000', 'white': 'FFFFFF', 'red': 'FF0000',
    'green': '008000', 'blue': '0000FF', 'yellow': 'FFFF00',
    'gray': '808080', 'grey': '808080', 'teal': '008080',
    'orange': 'FFA500', 'purple': '800080', 'maroon': '800000',
    'fuchsia': 'FF00FF', 'lime': '00FF00', 'aqua': '00FFFF',
    'navy': '000080', 'silver': 'C0C0C0', 'pink': 'FFC0CB',
    'brown': 'A52A2A', 'olive': '808000', 'darkblue': '00008B',
    'darkgreen': '006400', 'darkred': '8B0000', 'tan': 'D2B48C',
    'gold': 'FFD700', 'violet': 'EE82EE', 'indigo': '4B0082',
    'coral': 'FF7F50', 'salmon': 'FA8072', 'khaki': 'F0E68C',
    'plum': 'DDA0DD', 'wheat': 'F5DEB3', 'crimson': 'DC143C',
    'orchid': 'DA70D6', 'tomato': 'FF6347', 'linen': 'FAF0E6',
    'sienna': 'A0522D', 'bisque': 'FFE4C4', 'chocolate': 'D2691E',
    'cornsilk': 'FFF8DC', 'lavender': 'E6E6FA', 'snow': 'FFFAFA',
    'honeydew': 'F0FFF0', 'ivory': 'FFFFF0', 'mintcream': 'F5FFFA',
    'aliceblue': 'F0F8FF', 'azure': 'F0FFFF', 'beige': 'F5F5DC',
    'gainsboro': 'DCDCDC', 'lightgray': 'D3D3D3', 'lightgrey': 'D3D3D3',
    'darkgray': 'A9A9A9', 'darkgrey': 'A9A9A9', 'dimgray': '696969',
    'dimgrey': '696969', 'whitesmoke': 'F5F5F5', 'ghostwhite': 'F8F8FF',
    'floralwhite': 'FFFAF0', 'oldlace': 'FDF5E6', 'antiquewhite': 'FAEBD7',
    'papayawhip': 'FFEFD5', 'blanchedalmond': 'FFEBCD', 'moccasin': 'FFE4B5',
    'navajowhite': 'FFDEAD', 'peachpuff': 'FFDAB9', 'mistyrose': 'FFE4E1',
    'seashell': 'FFF5EE', 'lace': 'FFF0F5',
  };
  return map[name.toLowerCase()] || null;
}

function formatRuleColor(c) {
  if (!c || c === 'transparent') return undefined;
  if (/^[a-zA-Z]+$/.test(c) && c !== 'transparent') {
    const hex = namedColorToHex(c);
    if (hex) return hex;
    return undefined; // unknown named color → drop
  }
  // Monaco rules expect hex (no #), convert rgba/rgb to hex
  const h = normalizeHex(c);
  if (h) return h;
  return undefined;
}

function rgbaToHex(c) {
  const m = c.match(/rgba?\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)(?:\s*,\s*([\d.]+))?\s*\)/);
  if (!m) return null;
  const r = parseInt(m[1]).toString(16).padStart(2, '0');
  const g = parseInt(m[2]).toString(16).padStart(2, '0');
  const b = parseInt(m[3]).toString(16).padStart(2, '0');
  if (m[4] !== undefined) {
    const a = Math.round(parseFloat(m[4]) * 255).toString(16).padStart(2, '0');
    return '#' + r + g + b + a;
  }
  return '#' + r + g + b;
}

function formatChromeColor(c) {
  if (!c || c === 'transparent' || c === 'inherit') return undefined;
  // named colors → hex for safety (Monaco uses Color.fromHex internally)
  if (/^[a-zA-Z]+$/.test(c) && c !== 'transparent') {
    const hex = namedColorToHex(c);
    if (hex) return '#' + hex;
    return undefined;
  }
  // rgba/rgb → hex with # prefix (Monaco's Color.fromHex rejects CSS color strings, falls back to red)
  if (c.startsWith('rgba') || c.startsWith('rgb')) {
    const hex = rgbaToHex(c);
    if (hex) return hex;
    return undefined;
  }
  const h = normalizeHex(c);
  if (h) return '#' + h;
  return c;
}

// ── Main conversion ──

function convertTheme(css, themeName) {
  const blocks = parseCssRules(css);

  const classTokens = {}; // cm-class name → { foreground, background, fontStyle } (last-wins per class)
  const chrome = {};    // chrome color key → value
  let editorBg = null;

  for (const block of blocks) {
    const sel = block.selector;
    const props = extractColors(block.props);
    const cmClasses = extractCmClasses(sel);

    // Extract theme name from .cm-s-<name> selector
    const themeMatch = sel.match(/\.cm-s-([\w-]+)/);
    if (!themeMatch) continue;
    const selTheme = themeMatch[1];

    // Skip blocks disabled for this variant (e.g., solarized light/dark split)
    if (selTheme.includes('disabled') || sel.includes('disabled')) continue;

    // Detect editor background for dark/light detection
    if (sel.includes('CodeMirror') && !sel.includes('CodeMirror-')) {
      if (props.background && !editorBg) editorBg = props.background;
    }
    if (sel.match(/CodeMirror\s*$/)) {
      if (props.background && !editorBg) editorBg = props.background;
    }

    // ── Chrome colors (first-wins — the earliest declaration sets the color) ──

    for (const entry of CHROME_MAP) {
      if (entry.test.test(sel)) {
        if (entry.bg && props.background && !chrome[entry.bg]) chrome[entry.bg] = formatChromeColor(props.background);
        if (entry.fg && props.color && !chrome[entry.fg]) chrome[entry.fg] = formatChromeColor(props.color);
        if (entry.borderFg && props.borderColor && !chrome[entry.borderFg]) chrome[entry.borderFg] = formatChromeColor(props.borderColor);
      }
    }

    // Fallback: extract editor foreground/background from root .cm-s-<name> selectors
    // (some themes don't use .CodeMirror for the base text/background color)
    if (cmClasses.length === 0 && !selTheme.includes('disabled')) {
      if (!chrome['editor.foreground'] && props.color) {
        chrome['editor.foreground'] = formatChromeColor(props.color);
      }
      if (!chrome['editor.background'] && props.background) {
        chrome['editor.background'] = formatChromeColor(props.background);
      }
    }

    // ── Token colors (per CM-class, last-wins for same class) ──

    for (const cls of cmClasses) {
      const entry = TOKEN_MAP[cls];
      if (entry) {
        if (!classTokens[cls]) classTokens[cls] = {};
        if (props.color) classTokens[cls].foreground = formatRuleColor(props.color);
        if (props.background) classTokens[cls].background = formatRuleColor(props.background);
        if (props.fontStyle) classTokens[cls].fontStyle = props.fontStyle;
      }
    }
  }

  // Resolve CM classes → Monaco tokens (first-wins across different CM classes)
  const tokens = {};
  for (const [cls, vals] of Object.entries(classTokens)) {
    const tokenName = TOKEN_MAP[cls].token;
    if (!tokens[tokenName]) {
      tokens[tokenName] = {};
    }
    if (!tokens[tokenName].foreground && vals.foreground) tokens[tokenName].foreground = vals.foreground;
    if (!tokens[tokenName].background && vals.background) tokens[tokenName].background = vals.background;
    if (!tokens[tokenName].fontStyle && vals.fontStyle) tokens[tokenName].fontStyle = vals.fontStyle;
  }

  // Build rules array
  const rules = [];
  for (const [token, vals] of Object.entries(tokens)) {
    const rule = { token };
    if (vals.foreground) rule.foreground = vals.foreground;
    if (vals.background) rule.background = vals.background;
    if (vals.fontStyle) rule.fontStyle = vals.fontStyle;
    rules.push(rule);
  }

  // Detect base theme
  let dark;
  if (editorBg) {
    dark = isDark(editorBg);
  } else {
    // Fallback: use average luminance of token foregrounds
    // If tokens are mostly dark => light background; if mostly bright => dark background
    const luminances = [];
    for (const [, vals] of Object.entries(classTokens)) {
      if (vals.foreground) {
        const hex = normalizeHex(vals.foreground);
        if (hex) {
          const r = parseInt(hex.slice(0, 2), 16);
          const g = parseInt(hex.slice(2, 4), 16);
          const b = parseInt(hex.slice(4, 6), 16);
          luminances.push(0.299 * r + 0.587 * g + 0.114 * b);
        }
      }
    }
    if (luminances.length > 0) {
      const avg = luminances.reduce((a, b) => a + b, 0) / luminances.length;
      dark = avg >= 128; // bright tokens => dark bg, dark tokens => light bg
    } else {
      dark = true;
    }
  }

  // Build theme object
  const theme = {
    base: dark ? 'vs-dark' : 'vs',
    inherit: false,
    rules,
    colors: chrome,
  };

  return theme;
}

// ── Main ──

function main() {
  const args = process.argv.slice(2);
  let filterThemes = null;
  if (args.length > 0) {
    filterThemes = new Set(args.map(s => s.toLowerCase()));
  }

  if (!fs.existsSync(OUT_DIR)) {
    fs.mkdirSync(OUT_DIR, { recursive: true });
  }

  const files = fs.readdirSync(CM_THEME_DIR).filter(f => f.endsWith('.css'));
  const allThemes = [];

  for (const file of files) {
    const themeName = path.basename(file, '.css');
    if (filterThemes && !filterThemes.has(themeName.toLowerCase()) && !filterThemes.has(themeName)) {
      continue;
    }

    const css = fs.readFileSync(path.join(CM_THEME_DIR, file), 'utf-8');

    // Special handling: solarized has both dark and light variants
    if (themeName === 'solarized') {
      const darkCss = css.replace(/\.cm-s-solarized\.cm-s-light/g, '.cm-s-solarized-disabled')
                         .replace(/\.cm-s-light/g, '.cm-s-solarized-disabled')
                         .replace(/\.cm-s-dark/g, '');
      const darkTheme = convertTheme(darkCss, 'solarized-dark');
      const lightCss = css.replace(/\.cm-s-solarized\.cm-s-dark/g, '.cm-s-solarized-disabled')
                          .replace(/\.cm-s-dark/g, '.cm-s-solarized-disabled')
                          .replace(/\.cm-s-light/g, '');
      const lightTheme = convertTheme(lightCss, 'solarized-light');
      allThemes.push({ name: 'solarized-dark', theme: darkTheme }, { name: 'solarized-light', theme: lightTheme });
      continue;
    }

    const theme = convertTheme(css, themeName);
    allThemes.push({ name: themeName, theme });
  }

  // Write individual files
  for (const { name, theme } of allThemes) {
    const filePath = path.join(OUT_DIR, name + '.js');
    const content = `// Generated from ${name}.css\nmonaco.editor.defineTheme(${JSON.stringify(name)}, ${JSON.stringify(theme, null, 2)});\n`;
    fs.writeFileSync(filePath, content);
    console.log(`  ${name}.js`);
  }


}

main();
