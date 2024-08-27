#ifndef __ARQSIM_HEADER_OS_H__
#define __ARQSIM_HEADER_OS_H__

#include <cstdint>

#include <my-lib/std.h>
#include <my-lib/macros.h>

#include "config.h"
#include "arq-sim.h"

namespace OS {

// ---------------------------------------

void boot (Arch::Terminal *terminal, Arch::Cpu *cpu);

void interrupt (const Arch::InterruptCode interrupt);

void syscall ();

// ---------------------------------------

} // end namespace

#endif