#ifndef H_SHONDY_JSONUTIL
#define H_SHONDY_JSONUTIL

#include "json.hpp"
#include <iostream>

void writeJSONFile(const nlohmann::json &controlDataJSON,
                   const std::string &controlDataFilePath);

bool readJSONFile(nlohmann::json &readInJSONFile,
                  const std::string &JSONFilePath);

#endif // H_SHONDY_JSONUTIL
