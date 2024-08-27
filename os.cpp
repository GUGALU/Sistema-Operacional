#include <stdexcept>
#include <string>
#include <string_view>
#include <cstdint>
#include <cstdlib>
#include <array>
#include <thread>
#include <chrono>

#include "config.h"
#include "lib.h"
#include "arq-sim.h"
#include "os.h"

namespace OS {

// ---------------------------------------

enum ProcessStatus { exec, await, ready };

struct Process {
    int id;
    bool begin;
    std::string name;
    ProcessStatus status;
    uint16_t pc; // Program Counter
    std::array<uint16_t, 8> gprs; // General-purpose registers
    Process* next;
};

// ---------------------------------------

Arch::Terminal* t;
Arch::Cpu* c;
Process* current_process = nullptr;
std::string command_buffer = "";

// ---------------------------------------

void boot(Arch::Terminal* terminal, Arch::Cpu* cpu)
{
    terminal->println(Arch::Terminal::Type::Command, "Type commands here");
    terminal->println(Arch::Terminal::Type::App, "Apps output here");
    terminal->println(Arch::Terminal::Type::Kernel, "Kernel output here");

    t = terminal;
    c = cpu;
}

// ---------------------------------------

void interrupt(const Arch::InterruptCode interrupt)
{
    if (interrupt == Arch::InterruptCode::Keyboard) {
        char typed = t->read_typed_char();

        if (typed != '\0') {
            if (typed == '\b') {
                if (!command_buffer.empty()) {
                    command_buffer.pop_back();
                    t->print_str(Arch::Terminal::Type::Command, "\b \b");
                }
            } else {
                command_buffer += typed;
                t->print_str(Arch::Terminal::Type::Command, std::string(1, typed));
            }

            if (typed == '\n') {
                command_buffer.clear();
            }
            
            if (command_buffer == "/exit\n") {
                t->println(Arch::Terminal::Type::Kernel, "Shutting down...");
                std::this_thread::sleep_for(std::chrono::seconds(2));
                c->turn_off();
            } else {
                t->println(Arch::Terminal::Type::App, "Unknown command: " + command_buffer);
            }
            
        }
    }
}

// ---------------------------------------

void syscall()
{
    
}

} // end namespace OS
