#include "Patcher.hpp"
#include "../fs.hpp"
#include <nod/nod.hpp>
#include "../dol/DolFile.hpp"

using namespace fs;
using namespace std;

#define FATAL(msg) { fprintf(stderr, msg); return 1; }


int patch_iso(std::vector<std::string> args) {
  if (args.size() < 1) FATAL("Must provide at least one positional argument (the iso to patch)\n");

  path cwd = current_path();
  path toPatch = absolute(cwd / args[0]);
  path outFile = absolute(cwd / "prime-practice-mod.iso");
  path tmp = absolute(cwd / "tmp");
  remove_all(tmp);
  create_directories(tmp);

  {
    string_view lastFile;
    nod::ExtractionContext ctx{true, [&lastFile](string_view name, float progress) {
      if (lastFile != name) {
        lastFile = name;
        printf("Extracting... %.2f%% (%s)\n", progress * 100.f, name.data());
      }
    }};

    printf("Extracting %s\n", toPatch.string().c_str());
    bool isWii;
#ifdef _WIN32
    unique_ptr<nod::DiscBase> disc = nod::OpenDiscFromImage(toPatch.wstring(), isWii);
#else
    unique_ptr<nod::DiscBase> disc = nod::OpenDiscFromImage(toPatch.string(), isWii);
#endif
    if (!disc) FATAL("Failed to open disc\n");
    if (isWii) FATAL("Must be a copy of NTSC 0-00, not a Wii game\n");

#ifdef _WIN32
    disc->extractToDirectory(tmp.wstring(), ctx);
#else
    disc->extractToDirectory(tmp.string(), ctx);
#endif
  }

  {
    path defaultDol = absolute(tmp / "files" / "default.dol");
    path mainDol = absolute(tmp / "sys" / "main.dol");
    path defaultBak = absolute(tmp / "default.dol");
    path mainBak = absolute(tmp / "main.dol");
    path patchFile = absolute(cwd / "release" / "DolPatch.bin");

    printf("Backing up dol files\n");
    copy_file(defaultDol, defaultBak);
    copy_file(mainDol, mainBak);

    DolFile dol;
    dol.readFrom(defaultBak);
    dol.applyPatch(defaultBak, patchFile, defaultDol);
    dol.readFrom(mainBak);
    dol.applyPatch(mainBak, patchFile, mainDol);

    printf("Deleting backups\n");
    remove_all(mainBak);
    remove_all(defaultBak);
  }

  {
    path boot = absolute(tmp / "sys" / "boot.bin");
    printf("Patching boot.bin\n");
    FILE *fp = fopen(boot.string().c_str(), "a+b");
    if (fp) {
      fclose(fp);
      fp = fopen(boot.string().c_str(), "r+b");
    }
    string newName = "Metroid Prime Practice Mod";
    fseek(fp, 0x20, SEEK_SET);
    fwrite(newName.data(), newName.size(), 1, fp);
    fclose(fp);
  }

  {
    printf("Copying in mod files\n");
    copy_file(
      cwd / "release" / "Mod.rel",
      tmp / "files" / "Mod.rel"
    );
    copy_file(
      cwd / "release" / "mod.js",
      tmp / "files" / "mod.js"
    );
    copy_file(
      cwd / "release" / "opening_practice.bnr",
      tmp / "files" / "opening.bnr",
#ifdef __APPLE__
      copy_option::overwrite_if_exists
#else
      copy_options::overwrite_existing
#endif
    );
  }

  {
    printf("Repacking Prime\n");
    nod::SystemStringView lastFile;
    auto progFunc = [&lastFile](float progress, nod::SystemStringView name, size_t fileBytesXfered) {
      if (lastFile != name) {
        lastFile = name;
        printf("Repacking... %.2f%% (%s)\n", progress * 100.f, name.data());
      }
    };

#ifdef _WIN32
    nod::DiscBuilderGCN builder(outFile.wstring(), progFunc);
    auto ret = builder.buildFromDirectory(tmp.wstring());
#else
    nod::DiscBuilderGCN builder(outFile.string(), progFunc);
    auto ret = builder.buildFromDirectory(tmp.string());
#endif

    switch (ret) {
      case nod::EBuildResult::Success:
        break;
      case nod::EBuildResult::Failed:
        FATAL("Failed to build disc\n");
      case nod::EBuildResult::DiskFull:
        FATAL("Failed to build disc: Resulting disc was too large\n");
    }
  }

  printf("Done!\n");
  return 0;
}
