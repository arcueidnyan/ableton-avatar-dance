autowatch = 1;
outlets = 1;

mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;

var song = null;
var frames = [];
var playing = false;
var beat = 0;
var stepsPerBeat = 4;
var lastFrame = -1;

function init() {
    song = new LiveAPI("live_set");
    bang();
}

function frameIndex(position, steps, count) {
    var index = Math.floor(position * steps);
    return ((index % count) + count) % count;
}
frameIndex.local = 1;

function bang() {
    if (!song || !song.id) return;
    var wasPlaying = playing;
    playing = Number(song.get("is_playing")) === 1;
    beat = playing ? Number(song.get("current_song_time")) : 0;
    if (!isFinite(beat)) beat = 0;

    var index = frameIndex(beat, stepsPerBeat, frames.length || 1);
    if (index !== lastFrame || (!frames.length && (playing || wasPlaying))) {
        lastFrame = index;
        mgraphics.redraw();
    }
}

function steps(value) {
    stepsPerBeat = Math.max(1, Math.min(16, Math.round(value)));
    bang();
}

function status(message) {
    outlet(0, message);
}
status.local = 1;

function release(images) {
    for (var i = 0; i < images.length; i++) images[i].freepeer();
}
release.local = 1;

function builtin() {
    release(frames);
    frames = [];
    lastFrame = -1;
    status("Built-in avatar");
    bang();
    mgraphics.redraw();
}

function avatar(path) {
    if (typeof path !== "string" || !path.length) return;
    var folder;
    var names = [];
    var loaded = [];
    var filename;
    try {
        folder = new Folder(path);
        while (!folder.end) {
            if (/^\d{4}\.png$/i.test(folder.filename)) names.push(folder.filename);
            folder.next();
        }
        names.sort();
        if (!names.length || names.length > 128) {
            throw new Error("Choose a folder with 1–128 numbered PNGs");
        }
        var width;
        var height;
        for (var i = 0; i < names.length; i++) {
            filename = names[i];
            var image = new Image(folder.pathname.replace(/\/?$/, "/") + filename);
            loaded.push(image);
            var size = image.size;
            if (!size || size[0] < 1 || size[1] < 1 || size[0] > 512 || size[1] > 512) {
                throw new Error("Use PNGs up to 512 × 512 px");
            }
            if (i === 0) {
                width = size[0];
                height = size[1];
            } else if (size[0] !== width || size[1] !== height) {
                throw new Error("All frames must have the same size");
            }
        }
    } catch (error) {
        release(loaded);
        status(String(error.message || error));
        return;
    } finally {
        if (folder) folder.close();
    }

    release(frames);
    frames = loaded;
    lastFrame = -1;
    status(frames.length + " PNG frame(s) loaded");
    bang();
    mgraphics.redraw();
}

function notifydeleted() {
    release(frames);
}
notifydeleted.local = 1;

function fillRect(x, y, w, h, r, g, b) {
    mgraphics.set_source_rgba(r, g, b, 1);
    mgraphics.rectangle(x, y, w, h);
    mgraphics.fill();
}
fillRect.local = 1;

function robot() {
    var phase = beat * Math.PI * 2;
    var sway = Math.sin(phase) * 7;
    var lift = -Math.abs(Math.sin(phase)) * 9;
    fillRect(-26, 40, 52, 3, 0.19, 0.22, 0.33);
    mgraphics.translate(sway, lift);
    fillRect(-12, 24, 8, 15, 0.45, 0.43, 0.9);
    fillRect(4, 24, 8, 15, 0.45, 0.43, 0.9);
    fillRect(-17, 0, 34, 26, 0.55, 0.5, 1);
    fillRect(-26, 4 + sway, 7, 16, 0.6, 0.56, 1);
    fillRect(19, 4 - sway, 7, 16, 0.6, 0.56, 1);
    fillRect(-22, -30, 44, 28, 0.73, 0.68, 1);
    fillRect(-5, -39, 4, 9, 0.73, 0.68, 1);
    fillRect(-7, -42, 8, 5, 0.49, 1, 0.74);
    fillRect(-14, -23, 28, 14, 0.12, 0.15, 0.27);
    fillRect(-10, -19, 5, 6, 0.49, 1, 0.74);
    fillRect(5, -19, 5, 6, 0.49, 1, 0.74);
}
robot.local = 1;

function paint() {
    var rect = box.rect;
    var width = rect[2] - rect[0];
    var height = rect[3] - rect[1];
    if (width <= 0 || height <= 0) return;
    fillRect(0, 0, width, height, 0.09, 0.1, 0.16);

    mgraphics.save();
    mgraphics.translate(width / 2, height / 2);
    if (frames.length) {
        var image = frames[Math.max(0, lastFrame)];
        var size = image.size;
        var scale = Math.min((width - 12) / size[0], (height - 12) / size[1]);
        if (scale > 0) {
            mgraphics.scale(scale, scale);
            mgraphics.translate(-size[0] / 2, -size[1] / 2);
            mgraphics.image_surface_draw(image);
        }
    } else {
        var ratio = Math.min(width / 130, height / 105);
        mgraphics.scale(ratio, ratio);
        robot();
    }
    mgraphics.restore();
}
