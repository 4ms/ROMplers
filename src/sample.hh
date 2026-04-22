#pragma once

#include <cstdint>
#include <span>

class Sample {
	std::span<const int16_t> data{};

public:
	constexpr Sample(auto &s)
		: data{s} {
	}

	float operator[](const uint32_t idx) const {
		return data[idx] * (1.f / 32768.f);
	}

	constexpr uint32_t size() const {
		return data.size();
	}
};
