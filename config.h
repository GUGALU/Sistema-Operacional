#ifndef __ARQSIM_HEADER_CONFIG_H__
#define __ARQSIM_HEADER_CONFIG_H__

#include <cstdint>

//#define CPU_DEBUG_MODE

namespace Config {

	inline constexpr uint32_t nregs = 8;

	inline constexpr uint16_t memsize_words = 1 << 15;

	inline constexpr uint32_t timer_interrupt_cycles = 1024;

}

#endif