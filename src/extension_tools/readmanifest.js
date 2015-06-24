var AdmZip = require('adm-zip');

var package = process.argv[2];

// Check args
if (process.argv.length !== 2+1) {
    console.error("Usage: " + process.argv[1] + " package");
    process.exit(1);
}

var MANIFEST_FILENAME = "nqq-manifest.json";

var zip = new AdmZip(package);
var manifest = zip.readAsText(MANIFEST_FILENAME);
console.log(manifest);