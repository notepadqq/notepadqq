/**
 * Debounce execution of a function. Debouncing, unlike throttling,
 * guarantees that a function is only executed a single time, either at the
 * very beginning of a series of calls, or at the very end.
 *
 * @param  {Number}   delay         A zero-or-greater delay in milliseconds. For event callbacks, values around 100 or 250 (or even higher) are most useful.
 * @param  {Boolean}  [atBegin]     Optional, defaults to false. If atBegin is false or unspecified, callback will only be executed `delay` milliseconds
 *                                  after the last debounced-function call. If atBegin is true, callback will be executed only at the first debounced-function call.
 *                                  (After the throttled-function has not been called for `delay` milliseconds, the internal counter is reset).
 * @param  {Function} callback      A function to be executed after delay milliseconds. The `this` context and all arguments are passed through, as-is,
 *                                  to `callback` when the debounced-function is executed.
 *
 * @return {Function} A new, debounced function.
 */
define(["./throttle"], function (_throttle) {
	"use strict";
  
	var exports = {};
	Object.defineProperty(exports, "__esModule", {
	  value: true
	});
  
	exports.default = function (delay, atBegin, callback) {
	  return callback === undefined ? (0, _throttle2.default)(delay, atBegin, false) : (0, _throttle2.default)(delay, callback, atBegin !== false);
	};
  
	var _throttle2 = _interopRequireDefault(_throttle);
  
	function _interopRequireDefault(obj) {
	  return obj && obj.__esModule ? obj : {
		default: obj
	  };
	}
  
	return exports.default;
});