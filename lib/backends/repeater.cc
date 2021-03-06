
/**
 * Source file for class graphite
 * Please see graphite.h and backend.h for documentaion
 */

#include "statsdcc/backends/repeater.h"

#include <iostream>
#include <unordered_map>

#include "statsdcc/os.h"
#include "statsdcc/net/wrapper.h"
#include "statsdcc/net/lib.h"
#include "statsdcc/configs/aggregator_config.h"

namespace statsdcc { namespace backends {

Repeater::Repeater() {
  // create write socket
  this->sockfd = net::wrapper::socket(AF_INET, SOCK_DGRAM, 0);

  // set destinations
  for (auto itr = config->backends.repeaters.cbegin();
       itr != config->backends.repeaters.cend();
       ++itr) {
    struct sockaddr_in dest_host;
    bzero(&dest_host, sizeof(dest_host));
    dest_host.sin_family = AF_INET;
    dest_host.sin_port = htons(itr->port);

    int ip_res =
      net::wrapper::inet_pton(AF_INET, itr->host.c_str(), &dest_host.sin_addr);

    if (ip_res == 0) {
      if (net::resolve_ip(itr->host.c_str(), &dest_host.sin_addr) == false) {
        throw "unable to resolve ip";
      }
    } else if (ip_res == -1) {
        throw "inet_pton error";
    }

    this->destinations.push_back(dest_host);
  }
}

void Repeater::flush_stats(const Ledger& ledger, int flusher_id) {
  std::string timer_data_key;

  // prefix for aggregator stats
  auto prefix = ::config->name + ".thread_" +
                  std::to_string(static_cast<long long int>(flusher_id));

  // counters
  for (auto counter_itr = ledger.counters.cbegin();
      counter_itr != ledger.counters.cend();
      ++counter_itr) {
    std::string key = counter_itr->first;
    std::string value =
      std::to_string(static_cast<long double>(counter_itr->second));

    if (::config->repeater_raw) {
      this->send(key + ":" + value + "|c");
    } else {
      std::string value_per_second =
        std::to_string(static_cast<long double>(ledger.counter_rates.at(key)));

      this->send(key + ".rate:" + value_per_second + "|c\n" +
                 key + ".count:" + value + "|c");
    }

  }

  // timers
  if (::config->repeater_raw) {
    for (auto timer_key_value_pair_itr = ledger.timers.cbegin();
         timer_key_value_pair_itr != ledger.timers.cend();
         ++timer_key_value_pair_itr) {

      std::string key = timer_key_value_pair_itr->first;
      std::string out = "";
      int out_limit = 20;
      int out_counter = 0;

      if (key.length() > 0) {
        std::vector<double> values(timer_key_value_pair_itr->second);
        for (auto value_itr = values.cbegin();
            value_itr != values.cend();
            ++value_itr) {

          out_counter++;
          std::string value = std::to_string(static_cast<long double>(*value_itr));
          out += key + ":" + value + "|ms\n";
          if (out_counter >= out_limit) {
            this->send(out);
            out = "";
            out_counter = 0;
          }
          
        }
        if(! out.empty()) {
            this->send(out);
        }
      }
    }
  } else {
    for (auto timer_itr = ledger.timer_data.cbegin();
        timer_itr != ledger.timer_data.cend();
        ++timer_itr) {
      std::string key = timer_itr->first;
      std::string out = "";

      for (auto timer_data_itr = timer_itr->second.cbegin();
          timer_data_itr != timer_itr->second.cend();
          ++timer_data_itr) {
        std::string timer_data_key = timer_data_itr->first;

        std::string value = std::to_string(
          static_cast<long double>(timer_data_itr->second));

        out += key + "." + timer_data_key + ":" + value + "|ms";
      }
      out.erase(out.end() - 1);
      this->send(out);
    }
  }

  // gauges
  for (auto gauge_itr = ledger.gauges.cbegin();
      gauge_itr != ledger.gauges.cend();
      ++gauge_itr) {
    std::string key = gauge_itr->first;

    std::string value = std::to_string(
      static_cast<long double>(gauge_itr->second));

    this->send(key + ":" + value + "|g");
  }

  // sets
  for (auto set_itr = ledger.sets.cbegin();
      set_itr != ledger.sets.cend();
      ++set_itr) {
    std::string key = set_itr->first;
    auto value = set_itr->second;

    this->send(key + ".count:" +
           std::to_string(static_cast<long long int>(value.size())) + "|s");

  }

  // prefix for stats
  this->prefix_stats = ::config->name;

  // Statsd metrics
  auto metrics_processed = ledger.statsd_metrics.find("metrics_processed");
  if (metrics_processed != ledger.statsd_metrics.end()) {
    std::string processed_val =
        std::to_string(static_cast<long long int>(metrics_processed->second));
    this->send(this->prefix_stats + ".metrics_processed:" + processed_val + "|c");
  }
  auto processing_time = ledger.statsd_metrics.find("processing_time");
  if (processing_time != ledger.statsd_metrics.end()) {
    std::string time_val =
        std::to_string(static_cast<long long int>(processing_time->second));
    this->send(this->prefix_stats + ".processing_time:" + time_val + "|ms");
  }

}

}  // namespace backends
}  // namespace statsdcc
