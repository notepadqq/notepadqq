define(["./throttle", "./debounce"], function (_throttle, _debounce) {
	"use strict";
  
	var exports = {};
	Object.defineProperty(exports, "__esModule", {
	  value: true
	});
	exports.debounce = exports.throttle = undefined;
  
	var _throttle2 = _interopRequireDefault(_throttle);
  
	var _debounce2 = _interopRequireDefault(_debounce);
  
	function _interopRequireDefault(obj) {
	  return obj && obj.__esModule ? obj : {
		default: obj
	  };
	}
  
	exports.throttle = _throttle2.default;
	exports.debounce = _debounce2.default;
	return exports;
});