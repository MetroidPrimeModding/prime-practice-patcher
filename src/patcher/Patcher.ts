import {Arguments} from "yargs";
import * as child_process from "child_process";
import * as path from "path";
import * as fs from 'fs-extra';
import {logger} from "../Logger";
import {ReleaseInfo} from "../utils/ReleaseInfo";
import {decryptFile, encryptFile, hashFile, padToLength, randomHex, xorFiles} from "../utils/Crypto";
import * as os from "os";

export class Patcher {
  constructor(private argv: Arguments) {
  }

  async execute(): Promise<void> {
    const releaseDir = path.resolve(process.cwd(), './release');

    let releaseInfo: ReleaseInfo = fs.readJsonSync(
      path.resolve(releaseDir, './mod.json')
    );

    const tmpDir = path.resolve(process.cwd(), './tmp');
    fs.removeSync(tmpDir);
    fs.mkdirpSync(tmpDir);

    const iso = path.resolve(process.cwd(), this.argv.iso);
    const outputFile = path.resolve(process.cwd(),
      this.argv.outputFile.replace('%ver%', releaseInfo.version)
    );

    logger.info('Patching to prime practice mod:');
    logger.info('Input: ' + iso);
    logger.info('Output: ' + outputFile);

    let java = 'java';
    if (os.platform() == 'win32') {
      java = path.resolve(process.cwd(), './jre1.8.0_152/bin/java');
      logger.v('Windows detected; using bundled java');
    }

    logger.info('Extracting Prime');
    let extractDir = path.resolve(tmpDir, 'GM8E01');
    child_process.execSync([
        java,
        '-jar',
        path.resolve(process.cwd(), './patcher-0.1.1.jar'),
        'extract -fq -o ',
        extractDir,
        '-i',
        iso
      ].join(' '),
      {
        stdio: 'inherit',
      }
    );

    logger.info('Patching files');

    logger.v('Copying default.dol to tmp dir');
    fs.copyFileSync(
      path.resolve(extractDir, './default.dol'),
      path.resolve(tmpDir, './default.dol')
    );

    logger.v('Padding default.dol to length');
    padToLength(
      path.resolve(tmpDir, './default.dol'),
      releaseInfo.sizes.defaultModDol
    );

    logger.v('Encrypting default.dol');
    await encryptFile(path.resolve(tmpDir, './default.dol'), releaseInfo.keys.defaultDol);

    logger.v('Xoring encrypted default.dol and xor file');
    xorFiles(
      path.resolve(tmpDir, './default.dol.aes256'),
      path.resolve(releaseDir, './default.dol.aes256.xor.default_mod.dol.aes256'),
      path.resolve(tmpDir, './default_mod.dol.aes256')
    );

    logger.v('Decrypting default_mod.dol');
    await decryptFile(path.resolve(tmpDir, './default_mod.dol.aes256'), releaseInfo.keys.defaultModDol);

    logger.v('Checking hash of default_mod.dol');
    {
      const hash = await hashFile(path.resolve(tmpDir, './default_mod.dol'));
      if (hash != releaseInfo.hashes.defaultModDol) {
        logger.error('Failed to properly patch default_mod.dol! Hash mismatch!');
        process.exit(1);
      }
    }

    logger.v('Patching info.json');
    {
      let infoJsonPath = path.resolve(extractDir, './info.json');
      const infoJson = fs.readJsonSync(infoJsonPath);
      infoJson.discHeader.name = 'Metroid Prime Practice Mod';
      fs.writeJSONSync(infoJsonPath, infoJson, {spaces: 2});
    }

    logger.v('Copying mod files in');
    fs.copyFileSync(
      path.resolve(tmpDir, './default_mod.dol'),
      path.resolve(extractDir, './default.dol')
    );
    fs.copyFileSync(
      path.resolve(releaseDir, './mod.js'),
      path.resolve(extractDir, './mod.js')
    );
    fs.copyFileSync(
      path.resolve(releaseDir, './Mod.rel'),
      path.resolve(extractDir, './Mod.rel')
    );
    fs.copyFileSync(
      path.resolve(releaseDir, './opening_practice.bnr'),
      path.resolve(extractDir, './opening.bnr')
    );

    logger.info('Repacking Prime');
    child_process.execSync([
        java,
        '-jar',
        path.resolve(process.cwd(), './patcher-0.1.1.jar'),
        'repack -fq -o',
        outputFile,
        '-i',
        extractDir
      ].join(' '),
      {
        stdio: 'inherit',
      }
    );

    logger.info(`Output file ${outputFile} produced!`);
  }
}

