import {Arguments, Argv} from "yargs";
import {CommandBase} from "./CommandBase";
import {ReleaseBuilder} from "../release/ReleaseBuilder";
import {logger} from "../Logger";


export class ReleaseCommand implements CommandBase {
  command = 'release <ver>';
  describe = 'Produce a release';

  builder(yargs: Argv): Argv {
    return yargs
        .positional('ver', {
          describe: 'Version of the resulting release',
          type: 'string'
        })
      .option('nobuild', {
        type: 'boolean',
        describe: 'Do not run the script/native builds\n(assume they are already built)',
        default: false
      });
  }

  handler(argv: Arguments) {
    const releaseBuilder = new ReleaseBuilder(argv);
    releaseBuilder.execute().then(() => {
      logger.info('Release created!');
    }).catch((error) => {
      logger.error('Error producing release: ' + error);
      throw error;
    });
  }
}
