#include <iostream>
#include <string_view>

#include <my-lib/std.h>
#include <my-lib/macros.h>

#include "arq-sim.h"
#include "lib.h"

namespace Lib {

// ---------------------------------------

static uint32_t get_file_size_bytes (const std::string_view fname)
{
	FILE *fp;
	
	fp = fopen(fname.data(), "rb");
	
	mylib_assert_exception_msg(fp != nullptr, "cannot load file ", fname)

	fseek(fp, 0, SEEK_END);
	const int32_t bsize = ftell(fp);

	fclose(fp);

	return bsize;
}

// ---------------------------------------

uint32_t get_file_size_words (const std::string_view fname)
{
	const uint32_t bsize = get_file_size_bytes(fname);

	static_assert(sizeof(uint16_t) == 2);

	mylib_assert_exception_msg((bsize & 0x01) == 0, "file size of ", fname, " is not even")

	return bsize / sizeof(uint16_t);
}

// ---------------------------------------

static bool load_from_disk_to_buffer (const std::string_view fname, void *ptr, const uint32_t mem_size_bytes)
{
	FILE *fp;
	
	fp = fopen(fname.data(), "rb");
	
	if (fp == nullptr)
		return false;

	fseek(fp, 0, SEEK_END);
	const uint32_t bsize = ftell(fp);

	if (bsize > mem_size_bytes) {
		fclose(fp);
		return false;
	}
	
	rewind(fp);

	if (fread(ptr, 1, bsize, fp) != bsize) {
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}

// ---------------------------------------

std::vector<uint16_t> load_from_disk_to_16bit_buffer (const std::string_view fname)
{
	const int32_t file_size_words = Lib::get_file_size_words(fname);

	std::vector<uint16_t> buffer(file_size_words);

	if (!load_from_disk_to_buffer(fname, buffer.data(), file_size_words * sizeof(uint16_t)))
		throw Mylib::Exception(Mylib::build_str_from_stream("cannot load file ", fname));

	return buffer;
}

// ---------------------------------------

} // end namespace