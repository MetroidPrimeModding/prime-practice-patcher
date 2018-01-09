const Jimp = require('jimp');
const fs = require('fs-extra');
const argv = require('yargs').argv;
const path = require('path');

if (argv._.length != 2) {
  console.error('requires 2 args');
  return;
}


async function main() {
  // const inp = fs.readFileSync(path.resolve(process.cwd(), argv._[0]));
  const out = Buffer.alloc(0x1800);

// Total buffer size: 0x1800
// Total pixels: 0xC00
  const width = 0x60;
  const height = 0x20;
  const img = await Jimp.read(path.resolve(process.cwd(), argv._[0]));

  function encodeRGB5A3(r,g,b,a) {
    if ((a >> 5) === 0x7) {
      return 0x8000 |
        (r >> 3) << 10 |
        (g >> 3) << 5 |
        (b >> 3) << 0;
    } else {
      return (a >> 5) << 12 |
        (r >> 4) << 8 |
        (g >> 4) << 4 |
        (b >> 4) << 0;
    }
  }

  let off = 0;
  for (let y = 0; y < height; y += 4) {
    for (let x = 0; x < width; x += 4) {
      // 4x4 blocks
      for (let iy = 0; iy < 4; iy++, off += 4) {
        for (let ix = 0; ix < 4; ix++) {
          const ind = img.getPixelIndex(x + ix, y + iy);
          const [r, g, b, a] = img.bitmap.data.slice(ind);
          const rgb5a3 = encodeRGB5A3(r, g, b, a);
          out.writeUInt16BE(rgb5a3, (off + ix) * 2);
        }
      }
    }
  }

  fs.writeFileSync(path.resolve(process.cwd(), argv._[1]), out);
}

main().then(() => {
  console.log('done');
});
