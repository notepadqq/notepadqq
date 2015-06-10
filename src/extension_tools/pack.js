var os = require('os');
var path = require('path');
var crypto = require('crypto');
var fs = require('fs');
var cp = require('child_process');

var ncp = require('ncp').ncp;
var mkdirp = require('mkdirp');
var rimraf = require('rimraf');
var archiver = require('archiver');

var project = process.argv[2];
var npm = process.argv[3];

var MANIFEST_FILENAME = "nqq-manifest.json";


var tmpdir = path.join(os.tmpdir(), '_notepadqq_ext_' + crypto.randomBytes(4).readUInt32LE(0));
var tmpdir_ext = path.join(tmpdir, 'ext');
var tmpdir_pkg = path.join(tmpdir, 'pkg');

mkdirp.sync(tmpdir);
mkdirp.sync(tmpdir_pkg);
mkdirp.sync(tmpdir_ext);

console.log(tmpdir);


function clean() {
    rimraf.sync(tmpdir);
}


function buildArchive(callback) {
    var temporaryArchive = path.join(tmpdir, "temp.zip");
    var output = fs.createWriteStream(temporaryArchive);
    var archive = archiver('zip');

    var definitiveArchive = "_____.zip";
    
    output.on('close', function() {
        console.log('Written ' + archive.pointer() + ' bytes');
        
        ncp(temporaryArchive, definitiveArchive, function (err) {
            if (err) {
                throw err;
            }
            
            callback(definitiveArchive);
        });
        
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
        definitiveArchive = files[0].replace(/(\.tar\.gz|\.[^\.]+)$/gi, "") + ".zip";
        archive.append(fs.createReadStream(path.join(tmpdir_pkg, files[0])), { name: files[0] });
    } else {
        throw "Unexpected files found in " + tmpdir_pkg;
    }

    archive.finalize();    
}



// Copy extension folder
ncp(project, tmpdir_ext, function (err) {
    if (err) {
        console.log("Error: " + err);
        clean();
        process.exit(1);
    }
    
    // Remove notepadqq files
    fs.unlinkSync(path.join(tmpdir_ext, MANIFEST_FILENAME));
    
    // FIXME Set bundleDependencies for the package
    //rewritePackageJSON(path.join('node_modules', packageName, 'package.json'));
    
    var npmPack = cp.spawn(npm, ['pack', tmpdir_ext], { cwd: tmpdir_pkg });
    
    npmPack.stdout.on('data', function (data) {
        console.log(data.toString());
    });

    npmPack.stderr.on('data', function (data) {
        console.error(data.toString());
    });

    npmPack.on('close', function (code) {
        if (code !== 0) {
            console.log("Error executing npm pack: exit code ", code);
            process.exit(0);
        }
        
        buildArchive(function(file) {
            console.log(file);
            clean();
        });
        
    });

});