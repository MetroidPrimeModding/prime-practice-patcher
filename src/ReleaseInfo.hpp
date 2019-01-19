#ifndef PRIME_PRACTICE_PATCHER_RELEASEINFO_HPP
#define PRIME_PRACTICE_PATCHER_RELEASEINFO_HPP

#include <string>
#include <nlohmann/json_fwd.hpp>

struct ReleaseInfo {
  std::string version;
  struct {
    std::string native;
    std::string script;
    std::string patcher;
  } revisions;
  struct {
    std::string defaultDol;
    std::string defaultModDol;
  } keys;
  struct {
    std::string defaultDol;
    std::string defaultModDol;
    std::string GM8E01;
  } hashes;
  struct {
    size_t defaultModDol;
  } sizes;

  void fromJson(nlohmann::json j);
  nlohmann::json toJson();
};

#endif //PRIME_PRACTICE_PATCHER_RELEASEINFO_HPP
