/**
 * Colors the native scrollbars of the editor page to match the active theme.
 *
 * Editor themes only style the editor surface, so Chromium keeps drawing its light default
 * scrollbars even on dark themes such as gruvbox-dark. The palette is derived from the computed
 * background of the themed element, which makes every theme work without per-theme CSS.
 */
var ScrollbarTheme = new function() {
    var STYLE_ELEMENT_ID = "nqq-scrollbar-theme";
    var SIZE_PX = 14;
    var DARK_LUMINANCE_THRESHOLD = 0.5;
    var DARK_TRACK_LIGHTEN = 20;
    var ARROW_PATHS = {
        up: "M4,9L7,4L10,9",
        down: "M4,5L7,10L10,5",
        left: "M9,4L4,7L9,10",
        right: "M5,4L10,7L5,10"
    };

    /**
     * Recolors the scrollbars once the theme is actually applied.
     *
     * The initial theme stylesheet is inserted dynamically by init.js and does not delay
     * DOMContentLoaded, so before the window load event the computed background may still be
     * the unthemed default. Afterwards a theme change is applied synchronously by the caller.
     *
     * @param themedElementSelector CSS selector of the element whose background the theme sets,
     *                              e.g. ".CodeMirror" or ".monaco-editor".
     */
    this.update = function(themedElementSelector) {
        if (document.readyState === "complete") {
            apply(themedElementSelector);
        } else {
            window.addEventListener("load", function() {
                apply(themedElementSelector);
            }, { once: true });
        }
    }

    /**
     * Writes the scrollbar stylesheet for the current background of the themed element.
     * Leaves the current scrollbar style untouched if the element or its color is unavailable.
     *
     * @param themedElementSelector See update().
     */
    function apply(themedElementSelector) {
        var element = document.querySelector(themedElementSelector);
        if (element === null) {
            return;
        }

        var rgb = parseRgb(window.getComputedStyle(element).backgroundColor);
        if (rgb.length === 0) {
            return;
        }

        writeStyle(buildCss(buildPalette(rgb)));
    }

    /**
     * @param color Computed CSS color, e.g. "rgb(40, 40, 40)".
     * @return [r, g, b] in 0..255, or an empty array for any other format (e.g. transparent rgba).
     */
    function parseRgb(color) {
        var match = color.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
        if (match === null) {
            return [];
        }

        return [parseInt(match[1], 10), parseInt(match[2], 10), parseInt(match[3], 10)];
    }

    /**
     * @param rgb Editor background as [r, g, b] in 0..255.
     * @return Colors for track, thumb, hovered thumb and arrows: a slightly lighter shade of the
     *         background on dark themes, the classic light gray scrollbar otherwise.
     */
    function buildPalette(rgb) {
        var luminance = (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]) / 255;
        if (luminance >= DARK_LUMINANCE_THRESHOLD) {
            return { track: "#f0f0f0", thumb: "rgba(0,0,0,0.25)", thumbHover: "rgba(0,0,0,0.4)", arrow: "#555" };
        }

        var track = rgb.map(function(channel) {
            return Math.min(channel + DARK_TRACK_LIGHTEN, 255);
        });
        return {
            track: "rgb(" + track.join(",") + ")",
            thumb: "rgba(255,255,255,0.2)",
            thumbHover: "rgba(255,255,255,0.35)",
            arrow: "#aaa"
        };
    }

    /**
     * Arrow glyph as an inline SVG. Drawn as an image because the CSS border-triangle trick
     * renders as a diamond inside scrollbar buttons.
     *
     * @param direction One of the keys of ARROW_PATHS.
     * @param color CSS color of the arrow.
     * @return CSS url() value.
     */
    function arrowImage(direction, color) {
        var svg = "<svg xmlns='http://www.w3.org/2000/svg' width='" + SIZE_PX + "' height='" + SIZE_PX + "'>"
            + "<path d='" + ARROW_PATHS[direction] + "' fill='" + color + "'/></svg>";
        return "url(\"data:image/svg+xml," + encodeURIComponent(svg) + "\")";
    }

    /**
     * @param palette Colors as returned by buildPalette().
     * @return Stylesheet text for the WebKit scrollbar pseudo-elements and CodeMirror's filler
     *         elements. Only one arrow button is shown at each end of a scrollbar.
     */
    function buildCss(palette) {
        var button = "::-webkit-scrollbar-button";
        return "::-webkit-scrollbar { width: " + SIZE_PX + "px; height: " + SIZE_PX + "px; }"
            + "::-webkit-scrollbar-track { background: " + palette.track + "; }"
            + "::-webkit-scrollbar-thumb { background: " + palette.thumb + "; border-radius: 7px;"
            + " border: 3px solid " + palette.track + "; }"
            + "::-webkit-scrollbar-thumb:hover { background: " + palette.thumbHover + "; }"
            + "::-webkit-scrollbar-corner, ::-webkit-resizer { background: " + palette.track + "; }"
            + button + " { display: none; }"
            + button + ":start:decrement, " + button + ":end:increment { display: block;"
            + " height: " + SIZE_PX + "px; width: " + SIZE_PX + "px; border: none;"
            + " background: " + palette.track + " no-repeat center / " + SIZE_PX + "px " + SIZE_PX + "px; }"
            + button + ":vertical:start:decrement { background-image: " + arrowImage("up", palette.arrow) + "; }"
            + button + ":vertical:end:increment { background-image: " + arrowImage("down", palette.arrow) + "; }"
            + button + ":horizontal:start:decrement { background-image: " + arrowImage("left", palette.arrow) + "; }"
            + button + ":horizontal:end:increment { background-image: " + arrowImage("right", palette.arrow) + "; }"
            + ".CodeMirror-scrollbar-filler, .CodeMirror-gutter-filler {"
            + " background: " + palette.track + " !important; }";
    }

    /**
     * Replaces the content of the page's single scrollbar stylesheet, creating it on first use.
     *
     * @param css Complete stylesheet text.
     */
    function writeStyle(css) {
        var style = document.getElementById(STYLE_ELEMENT_ID);
        if (style === null) {
            style = document.createElement("style");
            style.id = STYLE_ELEMENT_ID;
            document.head.appendChild(style);
        }

        style.textContent = css;
    }
}
