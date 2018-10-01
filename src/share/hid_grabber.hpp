#pragma once

// `krbn::hid_grabber` can be used safely in a multi-threaded environment.

#include "boost_utility.hpp"
#include "dispatcher.hpp"
#include "human_interface_device.hpp"
#include "logger.hpp"
#include "thread_utility.hpp"

namespace krbn {
class hid_grabber final : public pqrs::dispatcher::extra::dispatcher_client {
public:
  struct signal2_combiner_call_while_grabbable {
    typedef grabbable_state::state result_type;

    template <typename input_iterator>
    result_type operator()(input_iterator first_observer,
                           input_iterator last_observer) const {
      result_type value = grabbable_state::state::grabbable;
      for (;
           first_observer != last_observer && value == grabbable_state::state::grabbable;
           std::advance(first_observer, 1)) {
        value = *first_observer;
      }
      return value;
    }
  };

  // Signals

  boost::signals2::signal<grabbable_state::state(void), signal2_combiner_call_while_grabbable> device_grabbing;
  boost::signals2::signal<void(void)> device_grabbed;
  boost::signals2::signal<void(void)> device_ungrabbed;

  // Methods

  hid_grabber(const hid_grabber&) = delete;

  hid_grabber(std::weak_ptr<pqrs::dispatcher::dispatcher> weak_dispatcher,
              std::weak_ptr<human_interface_device> weak_hid) : dispatcher_client(weak_dispatcher),
                                                                weak_hid_(weak_hid),
                                                                enabled_(false),
                                                                grabbed_(false) {
    if (auto hid = weak_hid.lock()) {
      // opened
      {
        auto c = hid->opened.connect([this] {
          enqueue_to_dispatcher([this] {
            if (auto hid = weak_hid_.lock()) {
              grabbed_ = true;

              device_grabbed();

              hid->async_queue_start();
              hid->async_schedule();
            }
          });
        });
        human_interface_device_connections_.push_back(c);
      }

      // open_failed
      {
        auto c = hid->open_failed.connect([this](auto&& error) {
          enqueue_to_dispatcher([this, error] {
            if (auto hid = weak_hid_.lock()) {
              auto message = fmt::format("IOHIDDeviceOpen error: {0} ({1}) {2}",
                                         iokit_utility::get_error_name(error),
                                         error,
                                         hid->get_name_for_log());
              logger_unique_filter_.error(message);
            }
          });
        });
        human_interface_device_connections_.push_back(c);
      }

      // closed
      {
        auto c = hid->closed.connect([this] {
          enqueue_to_dispatcher([this] {
            if (auto hid = weak_hid_.lock()) {
              device_ungrabbed();
            }
          });
        });
        human_interface_device_connections_.push_back(c);
      }

      // close_failed
      {
        auto c = hid->close_failed.connect([this](auto&& error) {
          enqueue_to_dispatcher([this, error] {
            if (auto hid = weak_hid_.lock()) {
              auto message = fmt::format("IOHIDDeviceClose error: {0} ({1}) {2}",
                                         iokit_utility::get_error_name(error),
                                         error,
                                         hid->get_name_for_log());
              logger_unique_filter_.error(message);

              device_ungrabbed();
            }
          });
        });
        human_interface_device_connections_.push_back(c);
      }
    }
  }

  virtual ~hid_grabber(void) {
    detach_from_dispatcher([this] {
      ungrab();
    });

    // Disconnect `human_interface_device_connections_`

    if (auto hid = weak_hid_.lock()) {
      hid->get_run_loop_thread()->enqueue(^{
        human_interface_device_connections_.disconnect_all_connections();
      });
    } else {
      human_interface_device_connections_.disconnect_all_connections();
    }

    human_interface_device_connections_.wait_disconnect_all_connections();
  }

  std::weak_ptr<human_interface_device> get_weak_hid(void) {
    return weak_hid_;
  }

  grabbable_state::state make_grabbable_state(void) {
    return device_grabbing();
  }

  void async_grab(void) {
    enqueue_to_dispatcher([this] {
      enabled_ = true;

      logger_unique_filter_.reset();

      grab();
    });
  }

  void async_ungrab(void) {
    enqueue_to_dispatcher([this] {
      enabled_ = false;

      ungrab();
    });
  }

private:
  void grab(void) {
    if (!enabled_) {
      return;
    }

    if (grabbed_) {
      return;
    }

    if (!grabbed_) {
      if (auto hid = weak_hid_.lock()) {
        if (!hid->get_removed()) {
          switch (make_grabbable_state()) {
            case grabbable_state::state::grabbable:
              hid->async_open(kIOHIDOptionsTypeSeizeDevice);
              break;

            case grabbable_state::state::none:
            case grabbable_state::state::ungrabbable_temporarily:
            case grabbable_state::state::device_error:
              // Retry
              break;

            case grabbable_state::state::ungrabbable_permanently:
              return;
          }
        }
      }
    }

    enqueue_to_dispatcher(
        [this] {
          grab();
        },
        when_now() + std::chrono::milliseconds(1000));
  }

  void ungrab(void) {
    if (grabbed_) {
      if (auto hid = weak_hid_.lock()) {
        hid->async_unschedule();
        hid->async_queue_stop();
        hid->async_close();
      }

      grabbed_ = false;
    }
  }

  std::weak_ptr<human_interface_device> weak_hid_;

  boost_utility::signals2_connections human_interface_device_connections_;
  bool enabled_;
  bool grabbed_;
  logger::unique_filter logger_unique_filter_;
};
} // namespace krbn
