import * as crypto from "crypto";
import * as fs from 'fs-extra';
import * as path from "path";

export function randomHex(len: number): string {
  let buff = Buffer.alloc(len / 2);
  crypto.randomFillSync(buff);
  return buff.toString('hex');
}

export function encryptFile(file: string, pw: string): Promise<void> {
  return new Promise((resolve, reject) => {
    const outFile = file + '.aes256';
    const cipher = crypto.createCipher('aes256', pw);
    const input = fs.createReadStream(file);
    const output = fs.createWriteStream(outFile);

    output.on('finish', () => {
      resolve()
    });

    input.pipe(cipher).pipe(output);
  });
}

export function decryptFile(file: string, pw: string): Promise<void> {
  return new Promise((resolve, reject) => {
    const outFile = file + '.decrypt';
    const cipher = crypto.createDecipher('aes256', pw);
    const input = fs.createReadStream(file);
    const output = fs.createWriteStream(outFile);

    output.on('finish', () => {
      resolve()
    });

    input.pipe(cipher).pipe(output);
  });
}

export function xorFiles(filea: string, fileb: string) {
  const outfile = filea + '.xor.' + path.basename(fileb);
  const lena = fs.statSync(filea).size;
  const lenb = fs.statSync(fileb).size;
  if (lena != lenb) {
    throw new Error('Attempting to xor two files of different sizes');
  }

  const fda = fs.openSync(filea, 'r');
  const fdb = fs.openSync(fileb, 'r');
  const fdo = fs.openSync(outfile, 'w');

  const buffa = Buffer.alloc(0x1000);
  const buffb = Buffer.alloc(buffa.length);
  const buffo = Buffer.alloc(buffa.length);

  let read = 0;
  while ((read = fs.readSync(fda, buffa, 0, buffa.length, null)) > 0) {
    fs.readSync(fdb, buffb, 0, read, null);
    // If non-multiple-of-4, this keeps it safe
    // Technically XOR (len - read) bytes too many at the end, but that's fine
    for (let i = 0; i < buffa.length / 4; i++) {
      const a = buffa.readInt32LE(i * 4);
      const b = buffb.readInt32LE(i * 4);
      const o = a ^ b;
      buffo.writeInt32LE(o, i * 4);
    }
    fs.writeSync(fdo, buffo, 0, read, null);
  }

  fs.closeSync(fda);
  fs.closeSync(fdb);
  fs.closeSync(fdo);
}

export function hashFile(file: string): Promise<string> {
  return new Promise((resolve, reject) => {
    const hash = crypto.createHash('sha256');
    const input = fs.createReadStream(file);

    let res = '';
    hash.on('readable', () => {
      const data = hash.read();
      if (data) {
        if (data instanceof Buffer) {
          res += data.toString('hex');
        } else {
          res += data;
        }
      }
    });

    hash.on('finish', () => {
      resolve(res);
    });

    input.pipe(hash);
  });
}

export function padToLength(file: string, len: number) {
  const startSize = fs.statSync(file).size;
  const padLen = len - startSize;
  if (padLen < 0) {
    throw new Error('Attempting to pad a file to be shorter!');
  }
  const buff = Buffer.alloc(padLen, 0);
  const fd = fs.openSync(file, 'a');
  fs.writeSync(fd, buff, 0, padLen, startSize);
  fs.closeSync(fd);
}
