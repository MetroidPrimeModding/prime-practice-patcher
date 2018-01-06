import {Arguments} from "yargs";
import chalk from "chalk";
import * as child_process from "child_process";
import * as path from "path";
import * as fs from 'fs-extra';
import {logger} from "../Logger";

export class ReleaseBuilder {
  constructor(private argv: Arguments) {
  }

  execute() {
    logger.info(`Producing a a new release: version ${this.argv.ver}`);

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

    const scriptRev = this.getGitRevision(scriptPath);
    logger.v('Script revision: ' + scriptRev);

    const nativeRev = this.getGitRevision(nativePath);
    logger.v('Native revision: ' + nativeRev);


  }

  getGitRevision(dir: string): string {
    return child_process.execSync('git rev-parse HEAD', {
      cwd: dir,
      encoding: 'utf8',
    }).trim();
  }
}
