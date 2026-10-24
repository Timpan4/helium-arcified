#ifndef ARCIFIED_NATIVE_FEATURES_PORTABILITY_UTF8_H_
#define ARCIFIED_NATIVE_FEATURES_PORTABILITY_UTF8_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace arcified::portability::internal {

// Byte length of the first Unicode scalar, or failure for empty/invalid input.
// UTF-8 scalar encoding follows RFC 3629 section 3. NUL is valid here; each
// input boundary owns its additional text restrictions and error diagnostics.
inline std::optional<std::size_t> Utf8ScalarLength(std::string_view text) {
  if (text.empty())
    return std::nullopt;
  const auto lead = static_cast<unsigned char>(text.front());
  if (lead < 0x80)
    return 1;
  unsigned continuation_count;
  std::uint32_t scalar;
  std::uint32_t minimum;
  if (lead >= 0xc2 && lead <= 0xdf) {
    continuation_count = 1;
    scalar = lead & 0x1f;
    minimum = 0x80;
  } else if (lead >= 0xe0 && lead <= 0xef) {
    continuation_count = 2;
    scalar = lead & 0x0f;
    minimum = 0x800;
  } else if (lead >= 0xf0 && lead <= 0xf4) {
    continuation_count = 3;
    scalar = lead & 0x07;
    minimum = 0x10000;
  } else {
    return std::nullopt;
  }
  if (continuation_count > text.size() - 1)
    return std::nullopt;
  for (unsigned i = 1; i <= continuation_count; ++i) {
    const auto byte = static_cast<unsigned char>(text[i]);
    if ((byte & 0xc0) != 0x80)
      return std::nullopt;
    scalar = (scalar << 6) | (byte & 0x3f);
  }
  if (scalar < minimum || scalar > 0x10ffff ||
      (scalar >= 0xd800 && scalar <= 0xdfff)) {
    return std::nullopt;
  }
  return continuation_count + 1;
}

}  // namespace arcified::portability::internal

#endif  // ARCIFIED_NATIVE_FEATURES_PORTABILITY_UTF8_H_
