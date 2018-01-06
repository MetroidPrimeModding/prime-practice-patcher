import {Arguments, Argv} from "yargs";
import chalk from "chalk";
import {CommandBase} from "./CommandBase";

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
    console.log(chalk.cyan(`Patching ${argv.iso}`))
  }
}
