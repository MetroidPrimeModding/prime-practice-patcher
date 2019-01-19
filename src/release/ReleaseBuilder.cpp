#include "ReleaseBuilder.hpp"

#include <boost/filesystem.hpp>
#include "../ReleaseInfo.hpp"
#include <logvisor/logvisor.hpp>
#include "../crypto/crypto.hpp"

#define FATAL(msg) { fprintf(stderr, msg); return 1; }

using namespace boost::filesystem;
using namespace std;

int build_release(std::vector<std::string> args) {
  if (args.size() < 1) {
    fprintf(stderr, "Must provide at least one positional argument (the length)\n");
    return 1;
  }
  ReleaseInfo releaseInfo;
  releaseInfo.version = args[0];

  printf("Producing a new release: version %s\n", releaseInfo.version.c_str());

  path cwd = current_path();

  path scriptPath = (cwd / ".." / "prime-practice-script").normalize();
  printf("Script directory: %s\n", scriptPath.string().c_str());
  if (!is_directory(scriptPath)) FATAL("Script directory does not exist!\n");

  path nativePath = (cwd / ".." / "prime-practice-native").normalize();
  printf("Native directory: %s\n", nativePath.string().c_str());
  if (!is_directory(nativePath)) FATAL("Native directory does not exist!\n");

  path isoPath = (cwd / "GM8E01.iso").normalize();
  printf("ISO Path: %s\n", isoPath.string().c_str());
  if (!is_regular_file(isoPath)) FATAL("ISO doesn't exist!\n");

  path buildDir = (cwd / "release" / releaseInfo.version).normalize();
  path tmpDir = (buildDir / "tmp").normalize();
  path releaseDir = (buildDir / ("prime-practice-" + releaseInfo.version)).normalize();
  path releaseResDir = (releaseDir / "release").normalize();
  printf("Output directory: %s\n", buildDir.string().c_str());
  printf("Temp directory: %s\n", tmpDir.string().c_str());
  printf("Release directory: %s\n", releaseDir.string().c_str());
  printf("Release res directory: %s\n", releaseResDir.string().c_str());

  remove_all(buildDir);
  create_directories(buildDir);
  create_directories(tmpDir);
  create_directories(releaseDir);
  create_directories(releaseResDir);

  // TODO: revisions

  // Generate keys
  releaseInfo.keys.defaultDol = genHexKey();
  releaseInfo.keys.defaultModDol = genHexKey();
  printf("Encryption keys (not private): %s :: %s",
         releaseInfo.keys.defaultDol.c_str(),
         releaseInfo.keys.defaultModDol.c_str());

  // yeah yeah yeah I know system is ultra insecure, this is ONLY used on packaging
  // TODO: don't use system and be secure
//  printf("Running script build\n");
//  path scriptBuildPath = (scriptPath / "compile_prod.sh").normalize();
//  current_path(scriptPath);
//  system(scriptBuildPath.string().c_str());
//  current_path(cwd);
//
//  printf("Running native build\n");
//  path nativeBuildPath = (nativePath / "compile.sh").normalize();
//  current_path(nativePath);
//  system(nativeBuildPath.string().c_str());
//  current_path(cwd);

  printf("Copying mod.js\n");
  copy_file(
    scriptPath / "dist" / "mod.js",
    tmpDir / "mod.js"
  );

  printf("Copying mod.rel\n");
  copy_file(
    nativePath / "build" / "Mod.rel",
    tmpDir / "Mod.rel"
  );

  printf("Copying default.dol\n");
  copy_file(
    nativePath / "default.dol",
    tmpDir / "default.dol"
  );

  printf("Copying default_mod.dol\n");
  copy_file(
    nativePath / "default_mod.dol",
    tmpDir / "default_mod.dol"
  );

  printf("Padding default.dol to length\n");
  resize_file(tmpDir / "default.dol", file_size(tmpDir / "default_mod.dol"));

  printf("Encrypting default.dol\n");
  if (encryptFile(
    tmpDir / "default.dol",
    tmpDir / "default.dol.scrambled",
    releaseInfo.keys.defaultDol
  )) FATAL("Failed to encrypt default.dol");

  printf("Encrypting default_mod.dol\n");
  if (encryptFile(
    tmpDir / "default_mod.dol",
    tmpDir / "default_mod.dol.scrambled",
    releaseInfo.keys.defaultModDol
  )) FATAL("Failed to encrypt default_mod.dol");;

  printf("Xoring encrypted default.dol and encrypted default_mod.dol\n");
  if (xorFiles(
    tmpDir / "default.dol.scrambled",
    tmpDir / "default_mod.dol.scrambled",
    tmpDir / "default.dol.scrambled.xor.default_mod.dol.scrambled"
  )) FATAL("Failed to xor default.dol and default_mod.dol");

  printf("Copying xor file\n");
  copy_file(
    tmpDir / "default.dol.scrambled.xor.default_mod.dol.scrambled",
    releaseResDir / "default.dol.scrambled.xor.default_mod.dol.scrambled"
  );

  printf("Copying mod.js\n");
  copy_file(
    tmpDir / "./mod.js",
    releaseResDir / "mod.js"
  );

  printf("Copying Mod.rel\n");
  copy_file(
    tmpDir / "Mod.rel",
    releaseResDir / "Mod.rel"
  );

  printf("Copying opening_practice.bn\n");
  copy_file(
    cwd / "res" / "opening_practice.bnr",
    releaseResDir / "opening_practice.bnr"
  );

  // TODO: copy built patcher

  return 0;
}
