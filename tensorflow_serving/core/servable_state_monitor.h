/* Copyright 2016 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_SERVING_CORE_SERVABLE_STATE_MONITOR_H_
#define TENSORFLOW_SERVING_CORE_SERVABLE_STATE_MONITOR_H_

#include <map>

#include "tensorflow_serving/core/servable_id.h"
#include "tensorflow_serving/core/servable_state.h"
#include "tensorflow_serving/util/event_bus.h"
#include "tensorflow_serving/util/optional.h"

namespace tensorflow {
namespace serving {

// A utility that listens to an EventBus<ServableState>, and keeps track of the
// state of each servable mentioned on the bus. The intended use case is to
// track the states of servables in a Manager.
//
// Offers an interface for querying the servable states. It may be useful as the
// basis for dashboards, as well as for testing a manager.
//
// IMPORTANT: You must create this monitor before arranging for events to be
// published on the event bus, e.g. giving the event bus to a Manager.
class ServableStateMonitor {
 public:
  using ServableName = string;
  using Version = int64;
  using VersionMap = std::map<Version, ServableState>;
  using ServableMap = std::map<ServableName, VersionMap>;

  explicit ServableStateMonitor(EventBus<ServableState>* bus);
  ~ServableStateMonitor() = default;

  // Returns the current state of one servable, or nullopt if that servable is
  // not being tracked.
  optional<ServableState> GetState(const ServableId& servable_id) const;

  // Returns the current states of all tracked versions of the given servable,
  // if any.
  VersionMap GetAllVersionStates(const string& servable_name) const;

  // Returns the current states of all tracked versions of all servables.
  ServableMap GetAllServableStates() const;

 private:
  // Handles a bus event.
  void HandleEvent(const ServableState& state);

  std::unique_ptr<EventBus<ServableState>::Subscription> bus_subscription_;

  mutable mutex mu_;

  // The current state of each servable version that has appeared on the bus.
  // (Entries are never removed, even when they enter state kEnd.)
  ServableMap states_ GUARDED_BY(mu_);

  TF_DISALLOW_COPY_AND_ASSIGN(ServableStateMonitor);
};

}  // namespace serving
}  // namespace tensorflow

#endif  // TENSORFLOW_SERVING_CORE_SERVABLE_STATE_MONITOR_H_
