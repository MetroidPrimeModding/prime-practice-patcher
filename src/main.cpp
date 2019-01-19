#include <iostream>
#include <string>
#include "logvisor/logvisor.hpp"
#include "nod/nod.hpp"
#include "release/ReleaseBuilder.hpp"
#include <sodium.h>

logvisor::Module Log("prime-practice-patcher");

using namespace std;

int main(int argc, char *argv[]) {
  logvisor::RegisterStandardExceptions();
  logvisor::RegisterConsoleLogger();

  Log.report(logvisor::Info, "Staring up prime-practice-patcher");

  if (sodium_init() == -1) {
    fprintf(stderr, "Failed to init libsodium");
    return 1;
  }

  vector<string> args;
  for (int i = 1; i < argc; i++) {
    args.push_back(argv[i]);
  }
  if (args.size() < 1) {
    fprintf(stderr, "Must provide a command:\n");
    fprintf(stderr, "release <version> - produce a release\n");
    fprintf(stderr, "patch <path_to_iso> - patch an iso\n");
    return 1;
  }

  string command = args[0];
  args.erase(args.begin()); // ya ya ya I know it's slow idc, it's like a 3 item list

  if (command == "release") {
    return build_release(args);
  } else {
    fprintf(stderr, "Unknown command '%s'\n", command.c_str());
    return 1;
  }
}

