#ifndef __ARQSIM_HEADER_LIB_H__
#define __ARQSIM_HEADER_LIB_H__

#include <sstream>
#include <vector>

#include <cstdint>

namespace Lib {

// ---------------------------------------

// raises Mylib::Exception in case of error
uint32_t get_file_size_words (const std::string_view fname);

// raises Mylib::Exception in case of error
std::vector<uint16_t> load_from_disk_to_16bit_buffer (const std::string_view fname);

// ---------------------------------------

}

#endif