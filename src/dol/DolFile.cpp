#include "DolFile.hpp"

using namespace std;
using namespace fs;

void patch_rel24(std::vector<uint8_t> &buff, uint32_t offset, uint32_t basePatchAddress, uint32_t targetAddress);
void patch_hi_lo(std::vector<uint8_t> &buff, uint32_t offsetHi, uint32_t offsetLo, uint32_t targetAddress);

DolFile::DolFile() {

}

bool DolFile::isPatched() {
  return textSecOffsets[2] != 0;
}

void fread_be_32(uint32_t *target, FILE *f) {
  fread(target, 4, 1, f);
  *target = ntohl(*target);
}

void fwrite_be_32(uint32_t value, FILE *f) {
  value = htonl(value);
  fwrite(&value, 4, 1, f);
}

void DolFile::readFrom(path dolFile) {
  FILE *fp = fopen(dolFile.string().c_str(), "rb");

  for (int i = 0; i < 7; i++) fread_be_32(&textSecOffsets[i], fp);
  for (int i = 0; i < 11; i++) fread_be_32(&dataSecOffsets[i], fp);
  for (int i = 0; i < 7; i++) fread_be_32(&textSecAddresses[i], fp);
  for (int i = 0; i < 11; i++) fread_be_32(&dataSecAddresses[i], fp);
  for (int i = 0; i < 7; i++) fread_be_32(&textSecSizes[i], fp);
  for (int i = 0; i < 11; i++) fread_be_32(&dataSecSizes[i], fp);
  fread_be_32(&bssAddress, fp);
  fread_be_32(&bssSize, fp);
  fread_be_32(&entryPoint, fp);

  fclose(fp);
}

#define U32_AT_ADDRESS(buff, offset) ((uint32_t*)(&buff.data()[offset]))

void DolFile::applyPatch(path dolFile, path patchFile, path outFile) {
  vector<uint8_t> patchData;
  {
    size_t patchFileSize = file_size(patchFile);
    size_t padding = ((patchFileSize + 31) & ~31) - patchFileSize;
    patchData.resize(patchFileSize + padding);
    FILE *fpPatch = fopen(patchFile.string().c_str(), "rb");
    fread(patchData.data(), patchFileSize, 1, fpPatch);
    fclose(fpPatch);
  }

  printf("Adding patch to %08X\n", p1_0_00_patch_address);

  // Relocate addresses in the patch code to account for the address it's placed at in memory
  patch_hi_lo(patchData, 0x04, 0x08, p1_0_00_patch_address + 0xFF);
  patch_hi_lo(patchData, 0x20, 0x24, p1_0_00_patch_address + 0xF0);
  patch_hi_lo(patchData, 0x40, 0x44, p1_0_00_patch_address + 0xE9);
  patch_hi_lo(patchData, 0x64, 0x68, p1_0_00_patch_address);
  patch_hi_lo(patchData, 0x74, 0x78, p1_0_00_patch_address + 0xFF);
  patch_hi_lo(patchData, 0x90, 0x94, p1_0_00_patch_address + 0xE9);
  patch_rel24(patchData, 0x2C, p1_0_00_patch_address, p1_0_00_addrDVDOpen);
  patch_rel24(patchData, 0x4C, p1_0_00_patch_address, p1_0_00_addrNew);
  patch_rel24(patchData, 0x70, p1_0_00_patch_address, p1_0_00_addrDVDReadAsyncPrio);
  patch_rel24(patchData, 0x88, p1_0_00_patch_address, p1_0_00_addrDVDClose);
  patch_rel24(patchData, 0x9C, p1_0_00_patch_address, p1_0_00_addrNew);
  patch_rel24(patchData, 0xAC, p1_0_00_patch_address, p1_0_00_addrOSLink);
  patch_rel24(patchData, 0xBC, p1_0_00_patch_address, p1_0_00_addrPPCSetFpIEEEMode);

  //Update DOL header for write
  textSecOffsets[2] = textSecOffsets[1] + textSecSizes[1];
  textSecSizes[2] = static_cast<uint32_t>(patchData.size());
  textSecAddresses[2] = p1_0_00_patch_address;

  FILE *srcFP = fopen(dolFile.string().c_str(), "rb");
  FILE *outFP = fopen(outFile.string().c_str(), "wb");

  for (int i = 0; i < 7; i++) {
    uint32_t offset = textSecOffsets[i];
    if (i > 2 && offset != 0) {
      offset += patchData.size();
    }
    fwrite_be_32(offset, outFP);
  }
  for (int i = 0; i < 11; i++) {
    uint32_t offset = dataSecOffsets[i];
    if (offset != 0) {
      offset += patchData.size();
    }
    fwrite_be_32(offset, outFP);
  }
  for (int i = 0; i < 7; i++) fwrite_be_32(textSecAddresses[i], outFP);
  for (int i = 0; i < 11; i++) fwrite_be_32(dataSecAddresses[i], outFP);
  for (int i = 0; i < 7; i++) fwrite_be_32(textSecSizes[i], outFP);
  for (int i = 0; i < 11; i++) fwrite_be_32(dataSecSizes[i], outFP);
  fwrite_be_32(bssAddress, outFP);
  fwrite_be_32(bssSize, outFP);
  fwrite_be_32(entryPoint, outFP);
  for (int i = 0; i < 7; i++) fwrite_be_32(0, outFP);

  // write section data
  for (int i = 0; i < 7; i++) {
    if (i == 2) {
      fwrite(patchData.data(), patchData.size(), 1, outFP);
    } else {
      fseek(srcFP, textSecOffsets[i], SEEK_SET);
      vector<uint8_t> secData;
      secData.resize(textSecSizes[i]);
      fread(secData.data(), secData.size(), 1, srcFP);

      if (i == 1) {
        //Patch code in the original game code to call the custom linker function
        patch_rel24(secData, 0x17B4, textSecAddresses[i], p1_0_00_patch_address + 0x10);
        // Also patch rel14 (which is unused) to act like rel32 (which we need because of GCC)
        uint32_t secAddr = textSecAddresses[i];
        uint32_t offset = p1_0_00_addrHandleRel14 - secAddr;

        *U32_AT_ADDRESS(secData, offset + 0x0) = htonl(0x60000000); //NOP
        *U32_AT_ADDRESS(secData, offset + 0x4) = htonl(0x60000000); //NOP
        *U32_AT_ADDRESS(secData, offset + 0x8) = htonl(0x901c0000); //STW r0, 0(r28)
      }

      fwrite(secData.data(), secData.size(), 1, outFP);
    }
  }

  // and data sections too
  for (int i = 0; i < 11; i++) {
    fseek(srcFP, dataSecOffsets[i], SEEK_SET);
    vector<uint8_t> secData;
    secData.resize(dataSecSizes[i]);
    fread(secData.data(), secData.size(), 1, srcFP);
    fwrite(secData.data(), secData.size(), 1, outFP);
  }
}


void patch_rel24(std::vector<uint8_t> &buff, uint32_t offset, uint32_t basePatchAddress, uint32_t targetAddress) {
  uint32_t instruction = ntohl(*U32_AT_ADDRESS(buff, offset)) & ~0x3FFFFFC;
  uint32_t relAddr = targetAddress - (basePatchAddress + offset);
  instruction |= (relAddr & 0x3FFFFFC);
  *U32_AT_ADDRESS(buff, offset) = htonl(instruction);
}

void patch_hi_lo(std::vector<uint8_t> &buff, uint32_t offsetHi, uint32_t offsetLo, uint32_t targetAddress) {
  uint32_t instrHi = ntohl(*U32_AT_ADDRESS(buff, offsetHi)) & 0xFFFF0000;
  uint32_t instrLo = ntohl(*U32_AT_ADDRESS(buff, offsetLo)) & 0xFFFF0000;
  uint32_t addrHi = (targetAddress & 0xFFFF0000);
  uint32_t addrLo = (targetAddress & 0x0000FFFF);

  if (addrLo >= 0x8000) {
    addrHi += 0x10000;
  }

  instrHi |= (addrHi >> 16);
  instrLo |= addrLo;

  *U32_AT_ADDRESS(buff, offsetHi) = htonl(instrHi);
  *U32_AT_ADDRESS(buff, offsetLo) = htonl(instrLo);
}

int patch_dol(std::vector<std::string> args) {
  if (args.size() < 2) {
    fprintf(stderr, "Must provide at least tw positional argument: inDol, outDol0\n");
    return 1;
  }

  path cwd = current_path();
  path inDol = (cwd / args[0]).normalize();
  path outDol = (cwd / args[1]).normalize();
  path patchFile = (cwd / "res" / "DolPatch.bin").normalize();

  DolFile dolFile;
  dolFile.readFrom(inDol);
  printf("Writing patched file to %s\n", outDol.string().c_str());
  dolFile.applyPatch(inDol, patchFile, outDol);
  printf("Done\n");


  return 0;
}
