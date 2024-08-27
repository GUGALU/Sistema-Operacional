# Sobre

Trabalho da disciplina de Sistemas Operacionais.
Criar um Sistema Operacional simulado para a arquitetura vista na disciplina de Arquitetura de Computadores.

---

## Sobre a arquitetura

Consultar no endereço do Assembler:
https://github.com/ehmcruz/arq-sim-assembler

---

## Dependências

Depende das seguintes bibliotecas:

- NCurses
- My-lib (https://github.com/ehmcruz/my-lib). O Makefile está configurado para buscar o projeto **my-lib** no mesmo diretório pai que este projeto.

---

# Guia no Linux (Ubuntu)

## Compilando no Linux

Pacotes:
- libncurses-dev

**make CONFIG_TARGET_LINUX=1**

## Rodando no Linux

**./arq-sim-so**

---

# Guia no Windows

## Compilando no Windows (usando MSYS2)

Pacotes:
- mingw-w64-ucrt-x86_64-ncurses

**make CONFIG_TARGET_WINDOWS=1**

## Rodando no Windows

Considerando o terminal do MSYS2:

**unset TERM**    
**./arq-sim-so.exe**
# Sistema-operacional
# Sistema-Operacional
