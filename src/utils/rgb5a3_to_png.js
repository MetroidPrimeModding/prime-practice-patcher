const Jimp = require('jimp');
const fs = require('fs-extra');
const argv = require('yargs').argv;
const path = require('path');

if (argv._.length != 2) {
  console.error('requires 2 args');
  return;
}

const inp = fs.readFileSync(path.resolve(process.cwd(), argv._[0]));

// Total buffer size: 0x1800
// Total pixels: 0xC00
const width = 0x60;
const height = 0x20;
const img = new Jimp(width, height);

// Thanks, Dolphin, for having a working decoder
const s_lut5to8 = [
  0x00, 0x08, 0x10, 0x18, 0x20, 0x29, 0x31, 0x39,
  0x41, 0x4A, 0x52, 0x5A, 0x62, 0x6A, 0x73, 0x7B,
  0x83, 0x8B, 0x94, 0x9C, 0xA4, 0xAC, 0xB4, 0xBD,
  0xC5, 0xCD, 0xD5, 0xDE, 0xE6, 0xEE, 0xF6, 0xFF
];

const s_lut4to8 = [
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
  0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
];

const s_lut3to8 = [
  0x00, 0x24, 0x48, 0x6D, 0x91, 0xB6, 0xDA, 0xFF
];

const bg_color = 0x00000000;

function decodeRGB5A3(val) {
  let r, g, b, a;
  if ((val & 0x8000) > 0) {
    r = s_lut5to8[(val >> 10) & 0x1f];
    g = s_lut5to8[(val >> 5) & 0x1f];
    b = s_lut5to8[(val) & 0x1f];
    a = 0xFF;
  } else {
    a = s_lut3to8[(val >> 12) & 0x7];
    r = s_lut4to8[(val >> 8) & 0xf];
    g = s_lut4to8[(val >> 4) & 0xf];
    b = s_lut4to8[(val) & 0xf];
  }
  return [r, g, b, a];
}

let off = 0;
for (let y = 0; y < height; y += 4) {
  for (let x = 0; x < width; x += 4) {
    // 4x4 blocks
    for (let iy = 0; iy < 4; iy++, off += 4) {
      for (let ix = 0; ix < 4; ix++) {
        const ind = img.getPixelIndex(x + ix, y + iy);
        const src = inp.readUInt16BE((off + ix) * 2);
        const [r, g, b, a] = decodeRGB5A3(src);
        img.bitmap.data[ind + 0] = r;
        img.bitmap.data[ind + 1] = g;
        img.bitmap.data[ind + 2] = b;
        img.bitmap.data[ind + 3] = a;
      }
    }
  }
}

img.write(path.resolve(process.cwd(), argv._[1]));

