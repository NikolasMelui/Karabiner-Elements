#include "libkrbn/libkrbn.h"
#include "application_launcher.hpp"
#include "constants.hpp"
#include "core_configuration/core_configuration.hpp"
#include "dispatcher_utility.hpp"
#include "json_utility.hpp"
#include "launchctl_utility.hpp"
#include "libkrbn/impl/libkrbn_components_manager.hpp"
#include "libkrbn/impl/libkrbn_cpp.hpp"
#include "process_utility.hpp"
#include "types.hpp"
#include "update_utility.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace {
std::unique_ptr<libkrbn_components_manager> libkrbn_components_manager_;
} // namespace

void libkrbn_initialize(void) {
  krbn::dispatcher_utility::initialize_dispatchers();

  if (!libkrbn_components_manager_) {
    libkrbn_components_manager_ = std::make_unique<libkrbn_components_manager>();
  }
}

void libkrbn_terminate(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_ = nullptr;
  }

  krbn::dispatcher_utility::terminate_dispatchers();
}

const char* libkrbn_get_distributed_notification_observed_object(void) {
  return krbn::constants::get_distributed_notification_observed_object();
}

const char* libkrbn_get_distributed_notification_console_user_server_is_disabled(void) {
  return krbn::constants::get_distributed_notification_console_user_server_is_disabled();
}

const char* libkrbn_get_distributed_notification_device_grabbing_state_is_changed(void) {
  return krbn::constants::get_distributed_notification_device_grabbing_state_is_changed();
}

const char* libkrbn_get_grabber_alerts_json_file_path(void) {
  return krbn::constants::get_grabber_alerts_json_file_path();
}

const char* libkrbn_get_devices_json_file_path(void) {
  return krbn::constants::get_devices_json_file_path();
}

const char* libkrbn_get_device_details_json_file_path(void) {
  return krbn::constants::get_device_details_json_file_path();
}

const char* libkrbn_get_manipulator_environment_json_file_path(void) {
  return krbn::constants::get_manipulator_environment_json_file_path();
}

const char* libkrbn_get_user_complex_modifications_assets_directory(void) {
  return krbn::constants::get_user_complex_modifications_assets_directory().c_str();
}

bool libkrbn_lock_single_application_with_user_pid_file(const char* _Nonnull pid_file_name) {
  return krbn::process_utility::lock_single_application_with_user_pid_file(pid_file_name);
}

void libkrbn_unlock_single_application(void) {
  krbn::process_utility::unlock_single_application();
}

void libkrbn_launchctl_manage_console_user_server(bool load) {
  krbn::launchctl_utility::manage_console_user_server(load);
}

void libkrbn_launchctl_restart_console_user_server(void) {
  krbn::launchctl_utility::restart_console_user_server();
}

void libkrbn_check_for_updates_in_background(void) {
  krbn::update_utility::check_for_updates_in_background();
}

void libkrbn_check_for_updates_stable_only(void) {
  krbn::update_utility::check_for_updates_stable_only();
}

void libkrbn_check_for_updates_with_beta_version(void) {
  krbn::update_utility::check_for_updates_with_beta_version();
}

void libkrbn_launch_event_viewer(void) {
  krbn::application_launcher::launch_event_viewer();
}

void libkrbn_launch_menu(void) {
  krbn::application_launcher::launch_menu();
}

void libkrbn_launch_preferences(void) {
  krbn::application_launcher::launch_preferences();
}

bool libkrbn_system_core_configuration_file_path_exists(void) {
  return pqrs::filesystem::exists(krbn::constants::get_system_core_configuration_file_path());
}

void libkrbn_get_key_code_name(char* buffer, size_t length, uint32_t key_code) {
  buffer[0] = '\0';
  if (auto name = krbn::types::make_key_code_name(krbn::key_code(key_code))) {
    strlcpy(buffer, name->c_str(), length);
  }
}

void libkrbn_get_consumer_key_code_name(char* buffer, size_t length, uint32_t consumer_key_code) {
  buffer[0] = '\0';
  if (auto name = krbn::types::make_consumer_key_code_name(krbn::consumer_key_code(consumer_key_code))) {
    strlcpy(buffer, name->c_str(), length);
  }
}

bool libkrbn_device_identifiers_is_apple(const libkrbn_device_identifiers* p) {
  if (p) {
    return libkrbn_cpp::make_device_identifiers(*p).is_apple();
  }
  return false;
}

// ============================================================
// version_monitor
// ============================================================

void libkrbn_enable_version_monitor(libkrbn_version_monitor_callback callback,
                                    void* refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_version_monitor(callback,
                                                        refcon);
  }
}

void libkrbn_disable_version_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_version_monitor();
  }
}

// ============================================================
// system_preferences_monitor
// ============================================================

void libkrbn_enable_system_preferences_monitor(libkrbn_system_preferences_monitor_callback _Nullable callback,
                                               void* _Nullable refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_system_preferences_monitor(callback,
                                                                   refcon);
  }
}

void libkrbn_disable_system_preferences_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_system_preferences_monitor();
  }
}

// ============================================================
// connected_devices_monitor
// ============================================================

void libkrbn_connected_devices_terminate(libkrbn_connected_devices** p) {
  if (p && *p) {
    delete reinterpret_cast<libkrbn_connected_devices_class*>(*p);
    *p = nullptr;
  }
}

size_t libkrbn_connected_devices_get_size(libkrbn_connected_devices* p) {
  if (auto c = reinterpret_cast<libkrbn_connected_devices_class*>(p)) {
    return c->get_connected_devices().get_devices().size();
  }
  return 0;
}

const char* libkrbn_connected_devices_get_descriptions_manufacturer(libkrbn_connected_devices* p, size_t index) {
  if (auto c = reinterpret_cast<libkrbn_connected_devices_class*>(p)) {
    const auto& devices = c->get_connected_devices().get_devices();
    if (index < devices.size()) {
      return devices[index].get_descriptions().get_manufacturer().c_str();
    }
  }
  return nullptr;
}

const char* libkrbn_connected_devices_get_descriptions_product(libkrbn_connected_devices* p, size_t index) {
  if (auto c = reinterpret_cast<libkrbn_connected_devices_class*>(p)) {
    const auto& devices = c->get_connected_devices().get_devices();
    if (index < devices.size()) {
      return devices[index].get_descriptions().get_product().c_str();
    }
  }
  return nullptr;
}

bool libkrbn_connected_devices_get_device_identifiers(libkrbn_connected_devices* p, size_t index, libkrbn_device_identifiers* device_identifiers) {
  if (auto c = reinterpret_cast<libkrbn_connected_devices_class*>(p)) {
    if (device_identifiers) {
      const auto& devices = c->get_connected_devices().get_devices();
      if (index < devices.size()) {
        auto identifiers = devices[index].get_identifiers();
        device_identifiers->vendor_id = static_cast<uint32_t>(identifiers.get_vendor_id());
        device_identifiers->product_id = static_cast<uint32_t>(identifiers.get_product_id());
        device_identifiers->is_keyboard = static_cast<uint32_t>(identifiers.get_is_keyboard());
        device_identifiers->is_pointing_device = static_cast<uint32_t>(identifiers.get_is_pointing_device());
        return true;
      }
    }
  }
  return false;
}

bool libkrbn_connected_devices_get_is_built_in_keyboard(libkrbn_connected_devices* p, size_t index) {
  if (auto c = reinterpret_cast<libkrbn_connected_devices_class*>(p)) {
    const auto& devices = c->get_connected_devices().get_devices();
    if (index < devices.size()) {
      return devices[index].get_is_built_in_keyboard();
    }
  }
  return 0;
}

bool libkrbn_connected_devices_get_is_built_in_trackpad(libkrbn_connected_devices* p, size_t index) {
  if (auto c = reinterpret_cast<libkrbn_connected_devices_class*>(p)) {
    const auto& devices = c->get_connected_devices().get_devices();
    if (index < devices.size()) {
      return devices[index].get_is_built_in_trackpad();
    }
  }
  return 0;
}

void libkrbn_enable_connected_devices_monitor(libkrbn_connected_devices_monitor_callback callback,
                                              void* refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_connected_devices_monitor(callback,
                                                                  refcon);
  }
}

void libkrbn_disable_connected_devices_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_connected_devices_monitor();
  }
}

// ============================================================
// frontmost_application_monitor
// ============================================================

void libkrbn_enable_frontmost_application_monitor(libkrbn_frontmost_application_monitor_callback callback,
                                                  void* refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_frontmost_application_monitor(callback,
                                                                      refcon);
  }
}

void libkrbn_disable_frontmost_application_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_frontmost_application_monitor();
  }
}

// ============================================================
// log_monitor
// ============================================================

void libkrbn_enable_log_monitor(libkrbn_log_monitor_callback callback,
                                void* refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_log_monitor(callback,
                                                    refcon);
  }
}

void libkrbn_disable_log_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_log_monitor();
  }
}
