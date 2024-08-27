#ifndef __ARQSIM_HEADER_ARQSIM_H__
#define __ARQSIM_HEADER_ARQSIM_H__

#include <array>
#include <vector>
#include <string>
#include <string_view>

#include <cstdint>

#if defined(CONFIG_TARGET_LINUX)
	#include <ncurses.h>
#elif defined(CONFIG_TARGET_WINDOWS)
	#include <ncurses/ncurses.h>
#else
	#error Untested platform
#endif

#include <my-lib/std.h>
#include <my-lib/macros.h>
#include <my-lib/matrix.h>
#include <my-lib/bit.h>

#include "config.h"
#include "lib.h"

namespace Arch {

// ---------------------------------------

enum class InterruptCode : uint16_t
{
	Keyboard,
	Timer,
	GPF
};

const char* InterruptCode_str (const InterruptCode code);

// ---------------------------------------

class VideoOutput
{
private:
	using MatrixBuffer = Mylib::Matrix<char, true>;

	WINDOW *win;

	MatrixBuffer buffer;

	// cursor position in the buffer
	uint32_t x;
	uint32_t y;

public:
	VideoOutput (const uint32_t xinit, const uint32_t xend, const uint32_t yinit, const uint32_t yend);
	~VideoOutput ();

	void print (const std::string_view str);
	void dump () const;

private:
	void roll ();
	void update ();
};

// ---------------------------------------

class Terminal
{
public:
	enum class Type {
		Arch,
		Kernel,
		Command,
		App,

		Count // must be the last one
	};

private:
	std::vector<VideoOutput> videos;
	int typed_char;
	bool has_char;

public:
	Terminal ();
	~Terminal ();

	void run_cycle ();

	inline int read_typed_char ()
	{
		this->has_char = false;
		return this->typed_char;
	}

	inline bool is_backspace (const int c)
	{
		return (c == KEY_BACKSPACE) || (c == 8) || (c == 127); // || '\b'
	}

	inline bool is_alpha (const int c)
	{
		return (c >= 'a') && (c <= 'z');
	}

	inline bool is_num (const int c)
	{
		return (c >= '0') && (c <= '9');
	}

	inline bool is_return (const int c)
	{
		return (c == '\n');
	}

	void print_str (const Type video, const std::string_view str)
	{
		this->videos[ std::to_underlying(video) ].print(str);
	}

	template <typename... Types>
	void print (const Type video, Types&&... vars)
	{
		const std::string str = Mylib::build_str_from_stream(vars...);
		this->print_str(video, str);
	}

	template <typename... Types>
	void println (const Type video, Types&&... vars)
	{
		this->print(video, vars..., '\n');
	}

	void dump (const Type video) const
	{
		this->videos[ std::to_underlying(video) ].dump();
	}
};

// ---------------------------------------

class Memory
{
private:
	std::array<uint16_t, Config::memsize_words> data;

public:
	Memory ();
	~Memory ();

	inline uint16_t* get_raw ()
	{
		return this->data.data();
	}

	inline uint16_t operator[] (const uint32_t paddr) const
	{
		mylib_assert_exception(paddr < this->data.size())
		return this->data[paddr];
	}

	inline uint16_t& operator[] (const uint32_t paddr)
	{
		mylib_assert_exception(paddr < this->data.size())
		return this->data[paddr];
	}

	void dump (const uint16_t init = 0, const uint16_t end = Config::memsize_words-1) const;
};

// ---------------------------------------

class Timer
{
private:
	uint32_t count = 0;

public:
	void run_cycle ();
};

// ---------------------------------------

class Cpu
{
private:
	std::array<uint16_t, Config::nregs> gprs;
	InterruptCode interrupt_code;
	bool has_interrupt = false;

	OO_ENCAPSULATE_SCALAR(uint16_t, pc)
	OO_ENCAPSULATE_SCALAR_INIT(uint16_t, vmem_paddr_init, 0)
	OO_ENCAPSULATE_SCALAR_INIT(uint16_t, vmem_paddr_end, Config::memsize_words-1)

	OO_ENCAPSULATE_SCALAR_INIT_READONLY(uint16_t, pmem_size_words, Config::memsize_words)

private:
	Memory& memory;

public:
	Cpu ();
	~Cpu ();

	void run_cycle ();
	void dump () const;

	inline uint16_t get_gpr (const uint8_t code) const
	{
		mylib_assert_exception(code < this->gprs.size())
		return this->gprs[code];
	}

	inline void set_gpr (const uint8_t code, const uint16_t v)
	{
		mylib_assert_exception(code < this->gprs.size())
		this->gprs[code] = v;
	}

	inline uint16_t pmem_read (const uint16_t paddr) const
	{
		return this->memory[paddr];
	}

	inline void pmem_write (const uint16_t paddr, const uint16_t value)
	{
		this->memory[paddr] = value;
	}

	bool interrupt (const InterruptCode interrupt_code);
	void force_interrupt (const InterruptCode interrupt_code);
	void turn_off ();

private:
	void execute_r (const Mylib::BitSet<16> instruction);
	void execute_i (const Mylib::BitSet<16> instruction);

	inline uint16_t vmem_read (const uint16_t vaddr)
	{
		const uint16_t paddr = vaddr + this->vmem_paddr_init;

		if (paddr > this->vmem_paddr_end) {
			this->force_interrupt(InterruptCode::GPF);
			return 0;
		}

		return this->pmem_read(paddr);
	}

	inline void vmem_write (const uint16_t vaddr, const uint16_t value)
	{
		const uint16_t paddr = vaddr + this->vmem_paddr_init;

		if (paddr > this->vmem_paddr_end) {
			this->force_interrupt(InterruptCode::GPF);
			return;
		}

		this->pmem_write(paddr, value);
	}
};

// ---------------------------------------

} // end namespace

#endif