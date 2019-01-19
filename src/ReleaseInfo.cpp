#include "ReleaseInfo.hpp"
#include <nlohmann/json.hpp>

void ReleaseInfo::fromJson(nlohmann::json j) {
  version = j["version"];

  auto jRevisions = j["revisions"];
  revisions.native = jRevisions["native"];
  revisions.script = jRevisions["script"];
  revisions.patcher = jRevisions["patcher"];

  auto jKeys = j["keys"];
  keys.defaultDol = jKeys["defaultDol"];
  keys.defaultModDol = jKeys["defaultModDol"];

  auto jHashes = j["hashes"];
  hashes.defaultDol = jHashes["defaultDol"];
  hashes.defaultModDol = jHashes["defaultModDol"];
  hashes.GM8E01 = jHashes["GM8E01"];

  auto jSizes = j["sizes"];
  sizes.defaultModDol = jSizes["defaultModDol"];
}

nlohmann::json ReleaseInfo::toJson() {
  nlohmann::json j;
  j["version"] = version;

  nlohmann::json jRevisions;
  jRevisions["native"] = revisions.native;
  jRevisions["script"] = revisions.script;
  jRevisions["patcher"] = revisions.patcher;
  j["revisions"] = jRevisions;


  nlohmann::json jKeys;
  jKeys["defaultDol"] = keys.defaultDol;
  jKeys["defaultModDol"] = keys.defaultModDol;
  j["keys"] = jKeys;

  nlohmann::json jHashes;
  jHashes["defaultDol"] = hashes.defaultDol;
  jHashes["defaultModDol"] = hashes.defaultModDol;
  jHashes["GM8E01"] = hashes.GM8E01;
  j["hashes"] = jHashes;


  nlohmann::json jSizes;
  jSizes["defaultModDol"] = sizes.defaultModDol;
  j["sizes"] = jSizes;

  return j;
}
