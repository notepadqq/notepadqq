var os = require('os');
var path = require('path');
var crypto = require('crypto');
var fs = require('fs');

var archiver = require('archiver');

var sh = require('shelljs');

var project = process.argv[2];
var npm = process.argv[3];

// Check args
if (process.argv.length !== 2+2) {
    console.error("Usage: " + process.argv[1] + " projectFolder npmExecutable");
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

function clean() {
    sh.rm('-rf', tmpdir);
}

function addBundleDepsToPackageJSON(fileName) {
    var contents = fs.readFileSync(fileName);
    var json = JSON.parse(contents);
    if (json.dependencies) {
        delete json.bundledDependencies; // Remove key with different spelling
        json.bundleDependencies = Object.keys(json.dependencies);
        fs.writeFileSync(fileName, JSON.stringify(json, null, 2));  
    }
}


var tmpdir = path.join(os.tmpdir(), '_notepadqq_ext_' + crypto.randomBytes(4).readUInt32LE(0));
var tmpdir_ext = path.join(tmpdir, 'ext');
var tmpdir_pkg = path.join(tmpdir, 'pkg');

sh.mkdir('-p', tmpdir);     // /tmp/_notepadqq_ext_xxxxx/
failtest(sh.error());
sh.mkdir('-p', tmpdir_pkg); // Contains the notepadqq extension files
failtest(sh.error());
sh.mkdir('-p', tmpdir_ext); // Contains a copy of the extension code
failtest(sh.error());

console.log(tmpdir);

function buildArchive(callback) {
    var temporaryArchive = path.join(tmpdir, "temp.zip");
    var output = fs.createWriteStream(temporaryArchive);
    var archive = archiver('zip');

    var definitiveArchive = "_____.nqqext";
    
    output.on('close', function() {
        console.log('Written ' + archive.pointer() + ' bytes');
        sh.cp('-f', temporaryArchive, definitiveArchive);
        failtest(sh.error());
        callback(definitiveArchive);
    });

    archive.on('error', function(err) {
        throw err;
    });

    archive.pipe(output);

    // Add manifest
    archive.append(fs.createReadStream(path.join(project, MANIFEST_FILENAME)), { name: MANIFEST_FILENAME });
    
    // Add package
    var files = fs.readdirSync(tmpdir_pkg);
    if (files.length == 1) {
        definitiveArchive = files[0].replace(/(\.tar\.gz|\.[^\.]+)$/gi, "") + ".nqqext";
        archive.append(fs.createReadStream(path.join(tmpdir_pkg, files[0])), { name: files[0] });
    } else {
        throw "Unexpected files found in " + tmpdir_pkg;
    }

    archive.finalize();    
}


function prepare_npm_package(callback) {
    
    // Remove notepadqq files
    sh.rm(path.join(tmpdir_ext, MANIFEST_FILENAME));
    failtest(sh.error());
    
    // npm install
    exec(npm, ['install'], { cwd: tmpdir_ext }, function(code) {
        if (code !== 0) {
            console.log("Error executing npm install: exit code ", code);
            process.exit(1);
        }
        
        addBundleDepsToPackageJSON(path.join(tmpdir_ext, 'package.json'));

        callback();
    });

}

function create_npm_package(callback) {
    
    // npm pack
    exec(npm, ['pack', tmpdir_ext], { cwd: tmpdir_pkg }, function(code) {
        if (code !== 0) {
            console.log("Error executing npm pack: exit code ", code);
            process.exit(1);
        }

        callback();
    });
    
}



// Copy extension folder to a temporary dir
sh.cp('-r', path.join(project, '*'), path.join(project, '.*'), tmpdir_ext);
failtest(sh.error());

prepare_npm_package(function() {

    create_npm_package(function() {

        buildArchive(function(file) {
            console.log(file);
            clean();
        });

    });

});
