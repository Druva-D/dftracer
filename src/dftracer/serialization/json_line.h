#ifndef DFTRACER_SERIALIZATION_JSON_LINE_H
#define DFTRACER_SERIALIZATION_JSON_LINE_H

#include <dftracer/core/enumeration.h>
#include <dftracer/core/typedef.h>
#include <dftracer/utils/configuration_manager.h>

#include <any>
#include <cstddef>
#include <iomanip>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace dftracer {
class JsonLines {
  bool include_metadata;
  HashType hostname_hash;

 public:
  JsonLines();
  size_t initialize(char *buffer, HashType hostname_hash);
  size_t data(char *buffer, int index, ConstEventNameType event_name,
              ConstEventNameType category, TimeResolution start_time,
              TimeResolution duration,
              std::unordered_map<std::string, std::any> *metadata,
              ProcessID process_id, ThreadID tid);
  size_t metadata(char *buffer, int index, ConstEventNameType name,
                  ConstEventNameType value, ConstEventNameType ph,
                  ProcessID process_id, ThreadID thread_id,
                  bool is_string = true);
  size_t finalize(char *buffer) {
    buffer[0] = ']';
    buffer[1] = '\n';
    return 0;
  }
};
}  // namespace dftracer

#endif  // DFTRACER_SERIALIZATION_JSON_LINE_H