/**
 * Reports the colors of the current editor theme to the application, so it can color the rest of the user interface.
 *
 * Background and foreground are read from the themed editor element. Selection and link colors depend on how the
 * editor engine exposes them, so the caller passes a function that returns them as CSS color strings; colors it
 * cannot find are left out and the application derives them itself.
 */
var ThemeColors = new function() {

    /**
     * @param themedElementSelector Selector of the element that carries the theme's background and text color.
     * @param readAccentColors Function receiving that element and returning { selection, link } as CSS color
     *                         strings; either may be an empty string.
     */
    this.report = function(themedElementSelector, readAccentColors) {
        if (document.readyState === "complete") {
            send(themedElementSelector, readAccentColors);
        } else {
            window.addEventListener("load", function() {
                send(themedElementSelector, readAccentColors);
            }, { once: true });
        }
    }

    function send(themedElementSelector, readAccentColors) {
        var element = document.querySelector(themedElementSelector);
        if (element === null) {
            return;
        }

        var style = window.getComputedStyle(element);
        var background = parseColor(style.backgroundColor, []);
        var foreground = parseColor(style.color, background);
        if (background.length === 0 || foreground.length === 0) {
            return;
        }

        var accents = readAccentColors(element);
        var colors = { background: background, foreground: foreground };
        var selection = parseColor(normalize(accents.selection), background);
        var link = parseColor(normalize(accents.link), background);
        if (selection.length > 0) {
            colors.selection = selection;
        }
        if (link.length > 0) {
            colors.link = link;
        }

        UiDriver.sendMessage("J_EVT_THEME_COLORS", colors);
    }

    /**
     * Turns any CSS color notation (hex, names, rgb, rgba) into the rgb()/rgba() form the browser computes.
     */
    function normalize(color) {
        if (!color) {
            return "";
        }

        var probe = document.createElement("span");
        probe.style.display = "none";
        probe.style.color = color.trim();
        document.body.appendChild(probe);
        var computed = probe.style.color === "" ? "" : window.getComputedStyle(probe).color;
        probe.remove();

        return computed;
    }

    /**
     * Parses rgb()/rgba() into [r, g, b]. A translucent color is composed onto @p underlay, the color it is drawn
     * on; without an underlay it is rejected, because its visible color is unknown.
     */
    function parseColor(color, underlay) {
        var match = color.match(/^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*([\d.]+))?\)$/);
        if (match === null) {
            return [];
        }

        var rgb = [parseInt(match[1], 10), parseInt(match[2], 10), parseInt(match[3], 10)];
        var alpha = match[4] === undefined ? 1 : parseFloat(match[4]);
        if (alpha >= 1) {
            return rgb;
        }
        if (underlay.length === 0) {
            return [];
        }

        return rgb.map(function(channel, index) {
            return Math.round(channel * alpha + underlay[index] * (1 - alpha));
        });
    }
}
