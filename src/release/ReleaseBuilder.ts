import {Arguments} from "yargs";
import * as child_process from "child_process";
import * as path from "path";
import * as fs from 'fs-extra';
import {logger} from "../Logger";
import {ReleaseInfo} from "../utils/ReleaseInfo";
import {encryptFile, hashFile, padToLength, randomHex, xorFiles} from "../utils/Crypto";

export class ReleaseBuilder {
  constructor(private argv: Arguments) {
  }

  async execute(): Promise<void> {
    let releaseInfo: Partial<ReleaseInfo> = {};
    releaseInfo.version = this.argv.ver;

    logger.info(`Producing a a new release: version ${releaseInfo.version}`);

    const scriptPath = path.resolve(process.cwd(), '../prime-practice-script');
    logger.v('Script directory: ' + scriptPath);
    if (!fs.existsSync(scriptPath)) {
      logger.error('Script directory does not exist!');
      process.exit(1);
    }

    const nativePath = path.resolve(process.cwd(), '../prime-practice-native');
    logger.v('Native directory: ' + nativePath);
    if (!fs.existsSync(nativePath)) {
      logger.error('Native directory does not exist!');
      process.exit(1);
    }

    const isoPath = path.resolve(process.cwd(), './GM8E01.iso');
    if (!fs.existsSync(isoPath)) {
      logger.error('Must provide GM8E01.iso in CWD');
      process.exit(1);
    }

    const buildDir = path.resolve(process.cwd(), `./release/${this.argv.ver}`);
    const tmpDir = path.resolve(buildDir, './tmp');
    const releaseDir = path.resolve(buildDir, `./prime-practice-${this.argv.ver}`);
    const releaseResDir = path.resolve(releaseDir, './release');
    logger.info('Output directory: ' + buildDir);
    logger.v('Temp directory: ' + tmpDir);
    logger.v('Release directory: ' + releaseDir);
    logger.v('Release res dir: ' + releaseResDir);
    fs.removeSync(buildDir);
    fs.mkdirpSync(buildDir);
    fs.mkdirpSync(tmpDir);
    fs.mkdirpSync(releaseDir);
    fs.mkdirpSync(releaseResDir);

    // Get repo revisions
    {
      const scriptRev = this.getGitRevision(scriptPath);
      logger.v('Script revision: ' + scriptRev);

      const nativeRev = this.getGitRevision(nativePath);
      logger.v('Native revision: ' + nativeRev);

      const patcherRev = this.getGitRevision(process.cwd());
      logger.v('Patcher revision: ' + patcherRev);

      releaseInfo.revisions = {
        script: scriptRev,
        native: nativeRev,
        patcher: patcherRev
      };
    }

    // Generate keys
    {
      releaseInfo.keys = {
        defaultDol: randomHex(32),
        defaultModDol: randomHex(32)
      };
      logger.v('default.dol encryption key: ' + releaseInfo.keys.defaultDol);
      logger.v('default_mod.dol encryption key: ' + releaseInfo.keys.defaultModDol);
    }

    if (!this.argv.nobuild) {
      logger.info('Running script build');
      child_process.execFileSync(path.resolve(scriptPath, "./compile_prod.sh"), {
        stdio: this.argv.verbose >= 1 ? 'inherit' : ['ignore', 'ignore', 'inherit'],
        cwd: scriptPath
      });

      logger.info('Running native build');
      child_process.execFileSync(path.resolve(nativePath, "./compile.sh"), {
        stdio: this.argv.verbose >= 1 ? 'inherit' : ['ignore', 'ignore', 'inherit'],
        cwd: nativePath,
        env: {
          'NOWATCH': 'true'
        }
      });

      logger.info('Running tsc on self');
      child_process.execSync('tsc', {
        stdio: this.argv.verbose >= 1 ? 'inherit' : ['ignore', 'ignore', 'inherit'],
        cwd: process.cwd()
      });

      logger.info('Running pkg on self');
      child_process.execSync(
        `pkg prime-practice-mod-patcher.js --out-path "${path.resolve(process.cwd(), './natives')}" --targets node10-win-x64,node10-macos-x64,node10-linux-x64`,
        {
          stdio: this.argv.verbose >= 1 ? 'inherit' : ['ignore', 'ignore', 'inherit'],
          cwd: './'
        }
      );
    }

    logger.v('Copying mod.js');
    fs.copyFileSync(
      path.resolve(scriptPath, './dist/mod.js'),
      path.resolve(tmpDir, './mod.js')
    );

    logger.v('Copying Mod.rel');
    fs.copyFileSync(
      path.resolve(nativePath, './build/Mod.rel'),
      path.resolve(tmpDir, './Mod.rel')
    );

    logger.v('Copying default.dol');
    fs.copyFileSync(
      path.resolve(nativePath, './default.dol'),
      path.resolve(tmpDir, './default.dol')
    );

    logger.v('Copying default_mod.dol');
    fs.copyFileSync(
      path.resolve(nativePath, './default_mod.dol'),
      path.resolve(tmpDir, './default_mod.dol')
    );

    logger.v('Padding default.dol to length');
    padToLength(
      path.resolve(tmpDir, './default.dol'),
      fs.statSync(path.resolve(tmpDir, './default_mod.dol')).size
    );

    logger.v('Encrypting default.dol');
    await encryptFile(path.resolve(tmpDir, './default.dol'), releaseInfo.keys.defaultDol);

    logger.v('Encrypting default.mod.dol');
    await encryptFile(path.resolve(tmpDir, './default_mod.dol'), releaseInfo.keys.defaultModDol);

    logger.v('Xoring encrypted default.dol and encrypted default_mod.dol');
    xorFiles(
      path.resolve(tmpDir, './default.dol.aes256'),
      path.resolve(tmpDir, './default_mod.dol.aes256')
    );

    logger.v('Copying xor file');
    fs.copyFileSync(
      path.resolve(tmpDir, './default.dol.aes256.xor.default_mod.dol.aes256'),
      path.resolve(releaseResDir, './default.dol.aes256.xor.default_mod.dol.aes256')
    );

    logger.v('Copying mod.js');
    fs.copyFileSync(
      path.resolve(tmpDir, './mod.js'),
      path.resolve(releaseResDir, './mod.js')
    );

    logger.v('Copying Mod.rel');
    fs.copyFileSync(
      path.resolve(tmpDir, './Mod.rel'),
      path.resolve(releaseResDir, './Mod.rel')
    );

    logger.v('Copying opening_practice.bnr');
    fs.copyFileSync(
      path.resolve(process.cwd(), './res/opening_practice.bnr'),
      path.resolve(releaseResDir, './opening_practice.bnr')
    );

    logger.v('Copying patcher');
    fs.copyFileSync(
      path.resolve(process.cwd(), './res/patcher-0.1.2.jar'),
      path.resolve(releaseDir, './patcher-0.1.2.jar')
    );

    {
      logger.v('Calculating hashes');
      const defaultDolHash = await hashFile(path.resolve(tmpDir, './default.dol'));
      const defaultModDolHash = await hashFile(path.resolve(tmpDir, './default_mod.dol'));
      // const GM8E01isoHash = await hashFile(path.resolve(process.cwd(), './GM8E01.iso'));
      // This won't change so
      const GM8E01isoHash = 'c49a86d7b61abdaa3fdd6966bed30a72fd28f18a0aa48c02f1d5269ab80ef68c';
      releaseInfo.hashes = {
        defaultDol: defaultDolHash,
        defaultModDol: defaultModDolHash,
        GM8E01: GM8E01isoHash
      }
    }

    {
      logger.v('Getting size of default_mod.dol');
      releaseInfo.sizes = {
        defaultModDol: fs.statSync(path.resolve(tmpDir, './default_mod.dol')).size
      }
    }

    logger.v('Saving mod info');
    fs.writeFileSync(
      path.resolve(releaseResDir, './mod.json'),
      JSON.stringify(releaseInfo, null, 2)
    );

    logger.v('Copying self in');
    fs.copyFileSync(
      path.resolve(process.cwd(), './package.json'),
      path.resolve(releaseDir, './package.json')
    );
    fs.copySync(
      path.resolve(process.cwd(), './natives'),
      releaseDir
    );
    fs.copyFileSync(
      path.resolve(process.cwd(), './patch.sh'),
      path.resolve(releaseDir, './patch.sh')
    );
    fs.copyFileSync(
      path.resolve(process.cwd(), './patch.bat'),
      path.resolve(releaseDir, './patch.bat')
    );
    fs.copyFileSync(
      path.resolve(process.cwd(), './README.md'),
      path.resolve(releaseDir, './README.md')
    );

    logger.v('Extracting windows version of java (ha, silly windows users)');
    child_process.execSync([
        'tar',
        '-xf',
        path.resolve(process.cwd(), './res/jre-8u152-windows-x64.tar.gz'),
        '-C',
        releaseDir
      ].map(v => `"${v}"`).join(' ')
    );

    logger.v('Creating release zip');
    child_process.execSync([
        'zip',
        '-r',
        path.resolve(buildDir, `prime-practice-${this.argv.ver}.zip`),
        path.relative(buildDir, releaseDir)
      ].map(v => `"${v}"`).join(' '),
      {
        cwd: buildDir
      }
    );
  }

  getGitRevision(dir: string): string {
    return child_process.execSync('git rev-parse HEAD', {
      cwd: dir,
      encoding: 'utf8',
    }).trim();
  }
}

