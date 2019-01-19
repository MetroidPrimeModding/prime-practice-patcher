#ifndef PRIME_PRACTICE_PATCHER_DOLFILE_HPP
#define PRIME_PRACTICE_PATCHER_DOLFILE_HPP

#include <cstdint>
#include <vector>
#include "../fs.hpp"

// In the original python, these were loaded from the LST
// Here, we hard-code them based on prime 1 0_00
constexpr uint32_t p1_0_00_addrDVDOpen = 0x80371684;
constexpr uint32_t p1_0_00_addrDVDReadAsyncPrio = 0x80371994;
constexpr uint32_t p1_0_00_addrDVDClose = 0x8037174C;
constexpr uint32_t p1_0_00_addrOSLink = 0x80382190;
constexpr uint32_t p1_0_00_addrPPCSetFpIEEEMode = 0x8036F8BC;
constexpr uint32_t p1_0_00_addrNew = 0x80315818;
constexpr uint32_t p1_0_00_addrHandleRel14 = 0x803820c0;
constexpr uint32_t p1_0_00_patch_address = 0x80002B00;

int patch_dol(std::vector<std::string> args);

class DolFile {
public:
  DolFile();

  void readFrom(fs::path dolFile);

  bool isPatched();

  void applyPatch(fs::path dolFile, fs::path patchFile, fs::path outFile);

private:
  uint32_t textSecOffsets[7];
  uint32_t dataSecOffsets[11];
  uint32_t textSecAddresses[7];
  uint32_t dataSecAddresses[11];
  uint32_t textSecSizes[7];
  uint32_t dataSecSizes[11];
  uint32_t bssAddress;
  uint32_t bssSize;
  uint32_t entryPoint;
};


#endif //PRIME_PRACTICE_PATCHER_DOLFILE_HPP
