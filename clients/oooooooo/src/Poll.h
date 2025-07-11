//
// Created by emb on 11/30/18.
//

#ifndef CRONE_POLL_H
#define CRONE_POLL_H

#include <lo/lo.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

namespace softcut_jack_osc {
class Poll {
 public:
  typedef std::function<void(const char *)> Callback;

  explicit Poll(std::string name) {
    std::ostringstream os;
    os << "/poll/" << name;
    path = os.str();
  }

  void setCallback(Callback c) { cb = std::move(c); }

  void start() {
    shouldStop = false;
    th = std::make_unique<std::thread>(std::thread([this] {
      while (!shouldStop) {
        this->cb(path.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(period));
      }
    }));
    th->detach();
  }

  void stop() {
    // in c++ there's no way to safely and non-cooperatively interrupt a thread.
    shouldStop = true;
    // i am reasonably sure this won't leak...
    th.reset();
  }

  void setPeriod(int ms) { period = ms; }

 private:
  Callback cb;
  std::atomic<int> period;
  std::atomic<bool> shouldStop;
  std::unique_ptr<std::thread> th;
  std::string path;
  lo_address addr;
};

}  // namespace softcut_jack_osc

#endif  // CRONE_POLL_H
