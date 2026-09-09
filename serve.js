// Server statico per provare la webapp in locale.
//
// python -m http.server non e' utilizzabile qui: gira in un sandbox che nega
// getcwd(), e http.server lo chiama a import time come default di --directory.
// Questo script non tocca mai process.cwd(): la root e' assoluta e fissa.
const http = require("http");
const fs = require("fs");
const path = require("path");

const ROOT = "/Users/eugenio.bellini/Desktop/MISC/WEB/claudio";
const PORT = 8777;

const TYPES = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".svg": "image/svg+xml",
  ".woff2": "font/woff2",
};

http
  .createServer((req, res) => {
    const url = decodeURIComponent((req.url || "/").split("?")[0]);
    const rel = url === "/" ? "index.html" : url.replace(/^\/+/, "");
    // path.resolve + prefix check: senza, un ".." nella URL leggerebbe
    // qualunque file del disco.
    const file = path.resolve(ROOT, rel);
    if (file !== ROOT && !file.startsWith(ROOT + path.sep)) {
      res.writeHead(403).end("forbidden");
      return;
    }
    fs.readFile(file, (err, data) => {
      if (err) {
        res.writeHead(404).end("not found");
        return;
      }
      res.writeHead(200, {
        "Content-Type": TYPES[path.extname(file).toLowerCase()] || "application/octet-stream",
        "Cache-Control": "no-store",
      });
      res.end(data);
    });
  })
  .listen(PORT, "127.0.0.1", () => console.log(`serving ${ROOT} on http://127.0.0.1:${PORT}`));
