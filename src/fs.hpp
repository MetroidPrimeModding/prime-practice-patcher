#ifndef PRIME_PRACTICE_PATCHER_FS_HPP
#define PRIME_PRACTICE_PATCHER_FS_HPP

#ifdef __APPLE__
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
#else
#include <filesystem>

namespace fs = std::filesystem;
#endif


#endif //PRIME_PRACTICE_PATCHER_FS_HPP
