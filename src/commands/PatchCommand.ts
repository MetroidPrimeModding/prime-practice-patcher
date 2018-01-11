import {Arguments, Argv} from "yargs";
import chalk from "chalk";
import {CommandBase} from "./CommandBase";
import {logger} from "../Logger";
import {ReleaseBuilder} from "../release/ReleaseBuilder";
import {Patcher} from "../patcher/Patcher";

export class PatchCommand implements CommandBase {
  command = 'patch <iso> [outputFile]';
  describe = 'Path an iso';

  builder(yargs: Argv): Argv {
    return yargs
        .positional('iso', {
          describe: 'ISO of Metroid Prime to patch',
          type: 'string'
        })
        .positional('outputFile', {
          describe: 'Output file',
          type: 'string',
          default: 'prime-practice-%ver%.iso'
        });
  }

  handler(argv: Arguments) {
    const patcher = new Patcher(argv);
    patcher.execute().then(() => {
      logger.info('Patched!');
    }).catch((error) => {
      logger.error('Error patching: ' + error);
      throw error;
    });
  }
}
