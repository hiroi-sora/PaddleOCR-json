const OCR = require('paddleocrjson');
const ocr = new OCR('PaddleOCR_json.exe', [], {
    cwd: 'D:\\www\\OCR\\PaddleOCR-json',
}, false);

ocr.once('init', console.log);
ocr.once('exit', process.exit);

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

app.route('/').get((req, res, next) =>
    res.send(`
<form action="/" method="POST" enctype="multipart/form-data">
    <input type="file" name="img">
    <button type="submit">Up</button>
</form>`
    )
).post((req, res, next) =>
    upload(req, res, (err) => {
        if (typeof err === 'undefined')
            return next();
        return next(err);
    })
).post(async (req, res, next) => {
    const image_path = req.file.path;
    ocr.flush({ image_path })
        .then((data) => {
            res.data = data;
            //console.log(data);
            return next();
        }).catch(() => {
            return next(500);
        }).finally(() => {
            fs.unlink(image_path).then();
        });
    return;
});
app.use((req, res, next) => {
    if (res.data === null)
        return next(404);
    if (typeof res.data === 'undefined')
        return next(405);
    return res.jsonp(res.data);
});
app.use((err, req, res, next) =>
    res.status(404).jsonp({
        code: -1,
        message: err,
        data: null
    })
);
const server = app.listen(3000, () =>
    console.log(server.address())
);
