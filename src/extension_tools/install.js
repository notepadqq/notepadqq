var path = require('path');
var fs = require('fs');

var AdmZip = require('adm-zip');

var sh = require('shelljs');

var package = process.argv[2];
var destdir = process.argv[3];
var npm = process.argv[4];

// Check args
if (process.argv.length !== 2+3) {
    console.error("Usage: " + process.argv[1] + " package destinationPath npmExecutable");
    process.exit(1);
}

var MANIFEST_FILENAME = "nqq-manifest.json";

function exec(name, args, options, callback) {
    var cp = require('child_process');
    var process = cp.spawn(name, args, options);

    process.stdout.on('data', function (data) { console.log(data.toString()); });
    process.stderr.on('data', function (data) { console.error(data.toString()); });

    process.on('close', function (code) {
        callback(code);
    });
}

function failtest(error) {
    if (error !== null)
        throw error;
}

var zip = new AdmZip(package);
var manifest = JSON.parse(zip.readAsText(MANIFEST_FILENAME));
var uniqueName = manifest.unique_name;

if (!uniqueName.match(/^[-_0-9a-z]+(\.[-_0-9a-z]+)+$/) || uniqueName.length > 64) {
	throw "Invalid unique name: " + uniqueName;
}

var extensionFolder = path.join(destdir, uniqueName);
var extensionFolderBackup = path.join(destdir, uniqueName + "%%BACKUP");

// Create backup copy if folder already existing
sh.rm('-rf', extensionFolderBackup);
sh.mv(extensionFolder, extensionFolderBackup);

try {
    
    sh.mkdir('-p', extensionFolder);
    failtest(sh.error());
    zip.extractAllTo(extensionFolder, true);

    // Find npm archive file
    var npmArchive = null;
    var files = fs.readdirSync(extensionFolder);
    if (files.length == 2) {
        for (var i = 0; i < files.length; i++) {
            if (files[i] !== MANIFEST_FILENAME) {
                npmArchive = path.join(extensionFolder, files[i]);
                break;
            }
        }
        if (npmArchive === null) {
            throw "npm tarball not found in " + extensionFolder;
        }
    } else {
        throw "Unexpected files found in " + extensionFolder;
    }

    var node_modules = path.join(extensionFolder, "node_modules");
    sh.mkdir('-p', node_modules);
    failtest(sh.error());
    exec(npm, ['install', '--production', '--no-registry', npmArchive], { cwd: extensionFolder }, function(code){
        if (code !== 0) throw "npm install failed: exit code " + code;

        // Take out the module from the node_modules folder
        // Rename node_modules to avoid conflicts with the inner node_modules that we're pulling out.
        var node_modules_tmp = path.join(extensionFolder, "___tmp_node_modules");
        sh.mv(node_modules, node_modules_tmp);
        failtest(sh.error());
        var folders = fs.readdirSync(node_modules_tmp);
        if (folders.length == 1) {
            sh.cp('-r', path.join(node_modules_tmp, folders[0], '*'), path.join(node_modules_tmp, folders[0], '.*'), extensionFolder);
            failtest(sh.error());

            // Clean all
            sh.rm('-rf', node_modules_tmp, npmArchive);
            failtest(sh.error());
            
            exec(npm, ['rebuild'], { cwd: extensionFolder }, function(code){
                if (code !== 0) throw "npm rebuild failed: exit code " + code;
                
                sh.rm('-rf', extensionFolderBackup);
                
                console.log("Installed to " + extensionFolder);
            });
            
        } else {
            throw "Unexpected files found in " + node_modules_tmp;
        }

    });
    
} catch (e) {
    console.error(e);
    console.log("Failed. Restoring backup.");
    sh.rm('-rf', extensionFolder);
    sh.mv(extensionFolderBackup, extensionFolder);
    process.exit(1);
}