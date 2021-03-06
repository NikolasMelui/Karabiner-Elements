#pragma once

#include <nlohmann/json.hpp>

namespace krbn {
enum class operation_type : uint8_t {
  none,
  // observer -> grabber
  grabbable_state_changed,
  caps_lock_state_changed,
  // console_user_server -> grabber
  connect_console_user_server,
  system_preferences_updated,
  frontmost_application_changed,
  input_source_changed,
  // grabber -> console_user_server
  shell_command_execution,
  select_input_source,
  end_,
};

NLOHMANN_JSON_SERIALIZE_ENUM(
    operation_type,
    {
        {operation_type::none, nullptr},
        {operation_type::grabbable_state_changed, "grabbable_state_changed"},
        {operation_type::caps_lock_state_changed, "caps_lock_state_changed"},
        {operation_type::connect_console_user_server, "connect_console_user_server"},
        {operation_type::system_preferences_updated, "system_preferences_updated"},
        {operation_type::frontmost_application_changed, "frontmost_application_changed"},
        {operation_type::input_source_changed, "input_source_changed"},
        {operation_type::shell_command_execution, "shell_command_execution"},
        {operation_type::select_input_source, "select_input_source"},
        {operation_type::end_, "end_"},
    });
} // namespace krbn
