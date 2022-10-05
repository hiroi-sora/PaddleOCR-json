const OCR = require('./OCR');
const ocr = new OCR('PaddleOCR_json.exe', [], {
    cwd: './PaddleOCR-json'
}, false);

const fs = require('fs').promises;

const mime = require('mime');
const imgMime = new Set(['bmp', 'jpeg', 'png', 'pbm', 'pgm', 'ppm', 'ras', 'tiff', 'exr', 'jp2'].map((v) => mime.getType(v)));

const app = require('express')();
const upload = require('multer')({
    dest: 'tmp',
    fileFilter(req, file, cb) {
        if (imgMime.has(file.mimetype))
            return cb(null, true);
        return cb('This format is not accepted');
    },
    limits: {
        fileSize: 10 * 1024 * 1024,
        files: 1,
    },
}).single('img');

app.route('/').get((req, res, next) => {
    res.send(`
<form action="/" method="POST" enctype="multipart/form-data">
    <input type="file" name="img">
    <button type="submit">Up</button>
</form>`);
    return;
}).post((req, res, next) => {
    upload(req, res, (err) => {
        if (typeof err === 'undefined')
            return next();
        return next(err);
    });
}).post(async (req, res, next) => {
    const path = req.file.path;
    ocr.postMessage({ image_dir: path })
        .then((data) => {
            res.data = data;
            //console.log(data);
            return next();
        }).catch(() => {
            return next(500);
        }).finally(() => {
            fs.unlink(path).then();
        });
    return;
});
app.use((req, res, next) => {
    if (res.data === null)
        return next(404);
    if (typeof res.data === 'undefined')
        return next(405);
    return res.jsonp(res.data);
}).use((err, req, res, next) => {
    return res.status(404).jsonp({
        code: -1,
        message: err,
        data: null
    });
});
const server = app.listen(3000, () => {
    console.log(server.address());
});