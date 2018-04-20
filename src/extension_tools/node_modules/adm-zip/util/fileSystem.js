exports.require = function() {
  return require(process.versions['electron'] ? "original-fs" : "fs");
};
