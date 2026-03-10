#pragma once

#include <cstdint>
#include <span>

class Sample {
  std::span<const uint8_t> data{};

public:
  constexpr Sample(auto &s) : data{s, sizeof(s)} {}

  float operator[](const uint32_t idx) const {
    const auto t = reinterpret_cast<const int16_t *>(data.data());
    return t[idx] / 32768.f;
  }

  constexpr uint32_t size() const { return data.size() / 2; }
};
