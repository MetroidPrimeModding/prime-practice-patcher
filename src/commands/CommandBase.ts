import {Arguments, Argv, CommandModule} from "yargs";

export interface CommandBase extends CommandModule {
  builder(yargs: Argv): Argv;

  handler(argv: Arguments): void;
}
