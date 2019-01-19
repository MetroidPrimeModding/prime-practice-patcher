#include "crypto.hpp"
#include <sodium.h>
#include <cmath>

using namespace std;
using namespace boost::filesystem;

string randomHex(size_t len) {
  unsigned char *srcBytes = new unsigned char[len / 2];
  randombytes_buf(srcBytes, len / 2);

  string res;
  res.resize(len + 1);
  sodium_bin2hex(res.data(), res.size(), srcBytes, len / 2);
  res.resize(len); // remove the null terminator

  return res;
}

vector<uint8_t> hexToBin(string hex) {
  vector<uint8_t> res;
  res.resize(hex.size() / 2);

  sodium_hex2bin(
    res.data(), res.size(),
    hex.c_str(), hex.size(),
    nullptr,
    nullptr, nullptr
  );

  return res;
}

// the encrypt/decrypt code here is basically ripped straight from libsodium's docs
// only reason I'm using libsodium at all is cuz it was easy to compile in
// ...this is unneeded in the world where I properly patch default.dol on the fly anyway
#define CHUNK_SIZE 4096

int encryptFile(path file, path outFile, string hexKey) {
  auto key = hexToBin(hexKey);
  if (key.size() != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
    fprintf(stderr, "Bad key length: %u not %u\n", key.size(), crypto_secretstream_xchacha20poly1305_KEYBYTES);
    return 1;
  }

  unsigned char buf_in[CHUNK_SIZE];
  unsigned char buf_out[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
  unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  crypto_secretstream_xchacha20poly1305_state state;
  FILE *outFilePtr, *inFilePtr;
  unsigned long long out_len;
  size_t rlen;
  int eof;
  unsigned char tag;

  inFilePtr = fopen(file.string().c_str(), "rb");
  outFilePtr = fopen(outFile.string().c_str(), "wb");
  crypto_secretstream_xchacha20poly1305_init_push(&state, header, key.data());
  fwrite(header, 1, sizeof(header), outFilePtr);
  do {
    rlen = fread(buf_in, 1, sizeof(buf_in), inFilePtr);
    eof = feof(inFilePtr);
    tag = eof ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0;
    crypto_secretstream_xchacha20poly1305_push(&state, buf_out, &out_len, buf_in, rlen,
                                               NULL, 0, tag);
    fwrite(buf_out, 1, (size_t) out_len, outFilePtr);
  } while (!eof);
  fclose(outFilePtr);
  fclose(inFilePtr);

  return 0;
}

int decryptFile(path file, path outFile, string hexKey) {
  auto key = hexToBin(hexKey);
  if (key.size() != crypto_secretstream_xchacha20poly1305_KEYBYTES) {
    fprintf(stderr, "Bad key length: %u not %u\n", key.size(), crypto_secretstream_xchacha20poly1305_KEYBYTES);
    return 1;
  }

  unsigned char buf_in[CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
  unsigned char buf_out[CHUNK_SIZE];
  unsigned char header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
  crypto_secretstream_xchacha20poly1305_state state;
  FILE *outFilePtr, *inFilePtr;
  unsigned long long out_len;
  size_t rlen;
  int eof;
  int ret = -1;
  unsigned char tag;

  inFilePtr = fopen(file.string().c_str(), "rb");
  outFilePtr = fopen(outFile.string().c_str(), "wb");
  fread(header, 1, sizeof(header), inFilePtr);
  if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key.data()) != 0) {
    fprintf(stderr, "Incomplete header\n");
    goto ret; /* incomplete header */
  }
  do {
    rlen = fread(buf_in, 1, sizeof buf_in, inFilePtr);
    eof = feof(inFilePtr);
    if (crypto_secretstream_xchacha20poly1305_pull(&state, buf_out, &out_len, &tag,
                                                   buf_in, rlen, NULL, 0) != 0) {
      fprintf(stderr, "Corrupted chunk\n");
      goto ret; /* corrupted chunk */
    }
    if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL && !eof) {
      fprintf(stderr, "Premature end of file\n");
      goto ret; /* premature end (end of file reached before the end of the stream) */
    }
    fwrite(buf_out, 1, (size_t) out_len, outFilePtr);
  } while (!eof);

  ret = 0;
  ret:
  fclose(outFilePtr);
  fclose(inFilePtr);
  return ret;
}

std::string genHexKey() {
  return randomHex(crypto_secretstream_xchacha20poly1305_KEYBYTES * 2);
}

int xorFiles(path fileA, path fileB,path outFile) {
  size_t sizeA = file_size(fileA);
  size_t sizeB = file_size(fileB);

  if (sizeA != sizeB) {
    fprintf(stderr, "Sizes don't match!");
    return 1;
  }

  FILE *fileAPtr = fopen(fileA.string().c_str(), "rb");
  FILE *fileBPtr = fopen(fileB.string().c_str(), "rb");
  FILE *outFilePtr = fopen(outFile.string().c_str(), "wb");

  constexpr size_t buffSize = 2048;
  char *aBuff = new char[buffSize];
  char *bBuff = new char[buffSize];
  char *cBuff = new char[buffSize];


  for (size_t read = 0; read < sizeA; read+= buffSize) {
    size_t toRead = min(buffSize, sizeA - read);
    fread(aBuff, toRead, 1, fileAPtr);
    fread(bBuff, toRead, 1, fileBPtr);

    for (int i = 0; i < buffSize; i++) {
      cBuff[i] = aBuff[i] ^ bBuff[i];
    }
    fwrite(cBuff, toRead, 1, outFilePtr);
  }

  delete[] aBuff;
  delete[] bBuff;
  delete[] cBuff;

  fclose(fileAPtr);
  fclose(fileBPtr);
  fclose(outFilePtr);

  return 0;
}
