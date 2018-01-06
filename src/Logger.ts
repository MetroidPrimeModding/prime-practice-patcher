import chalk from "chalk";

export class Logger {
  constructor(private verbosity = 0) {
  }

  setVerbosity(verbosity: number) {
    this.verbosity = verbosity;
  }

  info(msg: string) {
    console.log(chalk.cyan(msg))
  }

  v(msg: string) {
    if (this.verbosity >= 1) {
      console.log(chalk.gray(msg));
    }
  }

  vv(msg: string) {
    if (this.verbosity >= 2) {
      console.log(chalk.gray(msg));
    }
  }

  error(msg: string) {
    console.log(chalk.red(msg))
  }
}

export var logger = new Logger();
