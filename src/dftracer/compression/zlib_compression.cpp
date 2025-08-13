#include <dftracer/compression/zlib_compression.h>
#include <dftracer/core/singleton.h>
namespace dftracer {
template <>
std::shared_ptr<ZlibCompression> Singleton<ZlibCompression>::instance = nullptr;
template <>
bool Singleton<ZlibCompression>::stop_creating_instances = false;
}  // namespace dftracer