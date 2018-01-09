export interface ReleaseInfo {
  version: string;

  revisions: {
    native: string;
    script: string;
    patcher: string;
  }

  keys: {
    defaultDol: string;
    defaultModDol: string;
  }

  hashes: {
    defaultDol: string;
    defaultModDol: string;
  }
}
