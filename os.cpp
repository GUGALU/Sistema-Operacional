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
    uint16_t base_addr;  // Base address for virtual memory
    uint16_t limit_addr; // Limit address for virtual memory
  };

  Arch::Terminal *t;
  Arch::Cpu *c;
  Process *current_process = nullptr;
  std::string command_buffer = "";

  void processInit();
  void processCreate(std::string_view name, uint16_t pc);
  void processRun();
  void processStatus();
  void processDestroy();
  void syscall();
  void processSave();

  void boot(Arch::Terminal *terminal, Arch::Cpu *cpu)
  {
    terminal->println(Arch::Terminal::Type::Command, "Type commands here");
    terminal->println(Arch::Terminal::Type::App, "Apps output here");
    terminal->println(Arch::Terminal::Type::Kernel, "Kernel output here");

    t = terminal;
    c = cpu;

    // Initialize the first process
    processInit();

    // Create a new process
    processCreate("second", 0x0001);

    // Execute the process
    processRun();

    c->set_gpr(0, 1);      // Define syscall 1 (print string in the memory)
    c->set_gpr(1, 0x1000); // String address in the memory
    syscall();

    c->set_gpr(0, 2); // Define syscall 2 (new line)
    syscall();

    processStatus(); // Show process status
  }

  void interrupt(const Arch::InterruptCode interrupt)
  {
    if (interrupt == Arch::InterruptCode::Keyboard)
    {
      int typed = t->read_typed_char();

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

      if (t->is_backspace(typed))
      {
        if (!command_buffer.empty())
        {
          command_buffer.pop_back();
          for (size_t i = 0; i < command_buffer.size() + 1; ++i)
          {
            t->print(Arch::Terminal::Type::Command, '\r');
            t->print(Arch::Terminal::Type::Command, command_buffer);
          }
          t->print_str(Arch::Terminal::Type::Command, "\b \b");
        }
      }

      if (typed == '\n')
      {
        if (command_buffer.rfind("/syscall ", 0) == 0)
        {
          std::string syscall_num_str = command_buffer.substr(9); // Take the syscall number
          uint16_t syscall_num = std::stoi(syscall_num_str);

          c->set_gpr(0, syscall_num); // Syscall number
          // Add new syscalls here

          syscall();

          t->println(Arch::Terminal::Type::App, "Syscall " + std::to_string(syscall_num) + " executed.");
        }
        else if (command_buffer.rfind("/load ", 0) == 0) // Load a new process
        {
          std::string program_name = command_buffer.substr(6); // Extract the program name
          processCreate(program_name, 0x0001);                 // Load process
          t->println(Arch::Terminal::Type::Kernel, "Programa " + program_name + " carregado.");
        }
        else if (command_buffer == "/kill\n") // Kill process
        {
          if (current_process != nullptr)
          {
            t->println(Arch::Terminal::Type::Kernel, "Killing process " + current_process->name);
            processDestroy();
          }
          else
          {
            t->println(Arch::Terminal::Type::Kernel, "No process to kill.");
          }
        }
        else if (command_buffer == "/status\n") // Show process status
        {
          if (current_process != nullptr)
          {
            processStatus();
          }
          else
          {
            t->println(Arch::Terminal::Type::Kernel, "No process running.");
          }
        }
        else
        {
          t->println(Arch::Terminal::Type::App, "Unknown command: " + command_buffer);
        }

        command_buffer.clear();
      }
    }
  }

  void syscall()
  {
    const uint16_t syscall = c->get_gpr(0);
    uint16_t strAdr = c->get_gpr(1); // Address of the syscall

    // Verify if the address is valid
    if (strAdr < current_process->base_addr || strAdr >= current_process->limit_addr)
    {
      t->println(Arch::Terminal::Type::Kernel, "General Protection Fault: Invalid memory access.");
      return;
    }

    switch (syscall)
    {
    case 0:
      t->println(Arch::Terminal::Type::Kernel, "Shutting down...");
      std::this_thread::sleep_for(std::chrono::seconds(2));
      c->turn_off();
      break;
    case 1:
    {
      while (c->pmem_read(strAdr))
      {
        t->print(Arch::Terminal::Type::App, static_cast<char>(c->pmem_read(strAdr)));
        strAdr++;
      }
      break;
    }
    case 2:
      t->println(Arch::Terminal::Type::App);
      break;
    default:
      t->println(Arch::Terminal::Type::App, "Syscall desconhecida: " + std::to_string(syscall));
      break;
    }
  }

  void processInit()
  {
    // Carrega o tamanho do arquivo idle.bin
    uint32_t idle_bin_size = Lib::get_file_size_words("idle.bin");

    Process *p = new Process;
    p->id = 0;
    p->begin = true;
    p->name = "idle";
    p->status = ProcessStatus::exec;
    p->pc = 0;
    p->gprs.fill(0);

    p->base_addr = 0x1000;
    p->limit_addr = 0x1000 + idle_bin_size; // Aloca o limite baseado no tamanho do arquivo

    current_process = p;
  }

  void processCreate(std::string_view name, uint16_t pc)
  {
    Process *p = new Process;
    p->id = current_process->id + 1;
    p->begin = false;
    p->name = name;
    p->status = ProcessStatus::ready;
    p->pc = pc;
    p->gprs.fill(0);
    p->next = nullptr;

    p->base_addr = 0x1000;
    p->limit_addr = 0x2000;
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
      for (uint8_t i = 0; i < current_process->gprs.size(); ++i)
      {
        c->set_gpr(i, current_process->gprs[i]);
      }
      c->run_cycle();
    }
    else if (current_process->status == ProcessStatus::ready && current_process->next != nullptr)
    {
      processSave();
      current_process = current_process->next;
      current_process->status = ProcessStatus::exec;
      processRun();
    }
    else
    {
      current_process = current_process->next ? current_process->next : current_process;
      current_process->status = ProcessStatus::exec;
      processRun();
    }
  }

  void processSave()
  {
    current_process->pc = c->get_pc();
    for (uint8_t i = 0; i < current_process->gprs.size(); ++i)
    {
      current_process->gprs[i] = c->get_gpr(i);
    }
  }

  void processStatus()
  {
    if (current_process->status == ProcessStatus::exec)
    {
      t->println(Arch::Terminal::Type::Kernel, "Process " + current_process->name + " is running");
    }
    else if (current_process->status == ProcessStatus::ready)
    {
      t->println(Arch::Terminal::Type::Kernel, "Process " + current_process->name + " is ready");
    }

    t->println(Arch::Terminal::Type::Kernel, "Base Address: 0x" + std::to_string(current_process->base_addr));
    t->println(Arch::Terminal::Type::Kernel, "Limit Address: 0x" + std::to_string(current_process->limit_addr));
  }
} // end namespace OS
