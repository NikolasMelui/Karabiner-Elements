#include <catch2/catch.hpp>

#include "complex_modifications_assets_file.hpp"
#include <iostream>

TEST_CASE("lint") {
  std::ifstream assets_json_stream("json/lint/assets.json");
  auto assets_json = nlohmann::json::parse(assets_json_stream);
  for (const auto& assets_json_entry : assets_json) {
    std::vector<std::string> error_messages;
    try {
      auto file_path = "json/lint/" + assets_json_entry.at("input").get<std::string>();
      error_messages = krbn::complex_modifications_assets_file(file_path).lint();
    } catch (std::exception& e) {
      error_messages.push_back(e.what());
    }

    REQUIRE(error_messages == assets_json_entry.at("errors").get<std::vector<std::string>>());
  }
}
