import {Arguments, Argv} from "yargs";
import {CommandBase} from "./CommandBase";
import {ReleaseBuilder} from "../release/ReleaseBuilder";


export class ReleaseCommand implements CommandBase {
  command = 'release <ver>';
  describe = 'Produce a release';

  builder(yargs: Argv): Argv {
    return yargs
        .positional('ver', {
          describe: 'Version of the resulting release',
          type: 'string'
        });
  }

  handler(argv: Arguments) {
    const releaseBuilder = new ReleaseBuilder(argv);
    releaseBuilder.execute();
  }
}
