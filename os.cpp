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

namespace OS
{
  enum ProcessStatus
  {
    exec,
    ready
  };

  struct Process
  {
    uint16_t id;
    bool begin;
    std::string name;
    ProcessStatus status;
    uint16_t pc;                  // Program Counter
    std::array<uint16_t, 8> gprs; // General-purpose registers
    Process *next;
  };

  Arch::Terminal *t;
  Arch::Cpu *c;
  Process *current_process = nullptr;
  std::string command_buffer = "";

  // ---------------------------------------

  void boot(Arch::Terminal *terminal, Arch::Cpu *cpu)
  {
    terminal->println(Arch::Terminal::Type::Command, "Type commands here");
    terminal->println(Arch::Terminal::Type::App, "Apps output here");
    terminal->println(Arch::Terminal::Type::Kernel, "Kernel output here");

    t = terminal;
    c = cpu;
  }

  void interrupt(const Arch::InterruptCode interrupt)
  {
    if (interrupt == Arch::InterruptCode::Keyboard)
    {
      char typed = t->read_typed_char();

      if (typed != '\0')
      {
        if (typed == '\b')
        {
          if (!command_buffer.empty())
          {
            command_buffer.pop_back();
            t->print_str(Arch::Terminal::Type::Command, "\b \b");
          }
        }
        else
        {
          command_buffer += typed;
          t->print_str(Arch::Terminal::Type::Command, std::string(1, typed));
        }

        if (typed == '\n')
        {
          command_buffer.clear();
        }

        if (command_buffer == "/exit\n")
        {
          t->println(Arch::Terminal::Type::Kernel, "Shutting down...");
          std::this_thread::sleep_for(std::chrono::seconds(2));
          c->turn_off();
        }
        else
        {
          t->println(Arch::Terminal::Type::App, "Unknown command: " + command_buffer);
        }
      }
    }
  }

  // ---------------------------------------

  void syscall()
  {
    const uint16_t syscall = cpu->get_gpr(0);
    uint16_t strAdr = cpu->get_gpr(1);

    switch (syscall)
    {
    case 0:
      cpu->turn_off();
      break;
    case 1:
      while (cpu->pmem_read(strAdr))
      {
        uint16_t c = cpu->pmem_read(strAdr);
        t->print(Arch::Terminal::Type::App, static_cast<char>(c));
        strAdr++;
      }
      break;
    case 2:
      t->println(Arch::Terminal::Type::App);
      break;
    case 3:
      t->print(Arch::Terminal::Type::App, strAdr);
      break;
    }
  }

  void processInit()
  {
    Process *p = new Process;
    p->id = 0;
    p->begin = true;
    p->name = "first";
    p->status = ProcessStatus::exec;
    p->pc = 0;
    p->gprs.fill(0);
    p->next = nullptr;
    current_process = p;
  }

  void processCreate(std::string_view name, uint16_t pc)
  {
    Process *p = new Process;
    p->id = current_process->id + 1;
    p->begin = false;
    p->name = name;
    p->status = ProcessStatus::await;
    p->pc = pc;
    p->gprs.fill(0);
    p->next = nullptr;
    current_process->next = p;
  }

  void processDestroy()
  {
    Process *p = current_process;
    current_process = current_process->next;
    delete p;
  }

  void processRun()
  {
    if (current_process->status == ProcessStatus::exec)
    {
      c->set_pc(current_process->pc);
      c->set_gprs(current_process->gprs);
      c->run();
    }
  }

  void processSave()
  {
    current_process->pc = c->get_pc();
    current_process->gprs = c->get_gprs();
  }

  void processSt atus()
  {
    if (current_process->status == ProcessStatus::exec)
    {
      t->println(Arch::Terminal::Type::Kernel, "Process " + current_process->name + " is running");
    }
    else if (current_process->status == ProcessStatus::ready)
    {
      t->println(Arch::Terminal::Type::Kernel, "Process " + current_process->name + " is ready");
    }
  }

  void virtualMemory()
  {
    cpu->set_vmem_paddr_init(0x000);
    cpu->set_vmem_paddr_end(0xFFFF);
  }

} // end namespace OS
