#ifndef PRIME_PRACTICE_PATCHER_CRYPTO_HPP
#define PRIME_PRACTICE_PATCHER_CRYPTO_HPP

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

std::string randomHex(size_t len);

std::string genHexKey();

std::vector<uint8_t> hexToBin(std::string hex);

int encryptFile(boost::filesystem::path file, boost::filesystem::path outFile, std::string hexKey);

int decryptFile(boost::filesystem::path file, boost::filesystem::path outFile, std::string hexKey);

int xorFiles(boost::filesystem::path fileA, boost::filesystem::path fileB, boost::filesystem::path outFile);

#endif //PRIME_PRACTICE_PATCHER_CRYPTO_HPP
