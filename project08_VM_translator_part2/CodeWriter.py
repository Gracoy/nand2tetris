import os
import shutil
from VMConstant import *


class CodeWriter:
    def __init__(self, filename):
        self._vm_filename = ""
        self._output_dir = os.path.dirname(filename)
        self._output_filename = os.path.basename(filename)
        self._outfile = open(self._output_filename, 'w')

        self._label_num = 0
        self._function_num = 0

    def write_code(self, command_lines):
        for line in command_lines:
            self._outfile.write(f"// {' '.join(line[1]).replace('//', ';')}\n")
            cmd_type, command = line
            if cmd_type == C_ARITHMETIC:
                cmd = command[0]
                self.write_arithmetic(cmd)
            elif cmd_type == C_PUSH:
                seg, index = command[1], int(command[2])
                self.write_push(seg, index)
            elif cmd_type == C_POP:
                seg, index = command[1], int(command[2])
                self.write_pop(seg, index)
            elif cmd_type == C_LABEL:
                label = command[1]
                self.write_label(label)
            elif cmd_type == C_IF:
                label = command[1]
                self.write_if(label)
            elif cmd_type == C_GOTO:
                label = command[1]
                self.write_goto(label)
            elif cmd_type == C_FUNCTION:
                function, n_var = command[1], int(command[2])
                self.write_function(function, n_var)
            elif cmd_type == C_RETURN:
                self.write_return()
            elif cmd_type == C_CALL:
                function, n_arg = command[1], int(command[2])
                self.write_call(function, n_arg)

    def set_vm_filename(self, vm_file):
        self._vm_filename = vm_file.split('\\')[-1].replace('.vm', '').strip()

    def close_file(self):
        self._outfile.close()

    def move_file(self):
        shutil.move(self._output_filename, self._output_dir + '\\' + self._output_filename)

    def sys_init(self):
        self._outfile.write('// Set SP = 256\n')
        self._a_command(256)
        self._c_command('D', 'A')
        self._a_command('SP')
        self._c_command('M', 'D')
        self._outfile.write('// Define OS function\n')
        self.write_function('OS', 0)
        self._outfile.write('// Call Sys.init for runtime\n')
        self.write_call('Sys.init', 0)

    def write_arithmetic(self, cmd):
        if cmd == 'add' :
            self._a_command('SP')           # @SP
            self._c_command('AM', 'M-1')    # AM = M-1
            self._c_command('D', 'M')       # D = M
            self._a_command('SP')           # @SP
            self._c_command('A', 'M-1')     # A = M-1
            self._c_command('M', 'M+D')     # M = M+D
        elif cmd == 'sub' :
            self._a_command('SP')           # @SP
            self._c_command('AM', 'M-1')    # AM = M-1
            self._c_command('D', 'M')       # D = M
            self._a_command('SP')           # SP
            self._c_command('A', 'M-1')     # A = M-1
            self._c_command('M', 'M-D')     # M = M-D
        elif cmd == 'and' :
            self._a_command('SP')           # @SP
            self._c_command('AM', 'M-1')    # AM = M-1
            self._c_command('D', 'M')       # D = M
            self._a_command('SP')           # @SP
            self._c_command('A', 'M-1')     # A = M-1
            self._c_command('M', 'M&D')     # M = M&D
        elif cmd == 'or'  :
            self._a_command('SP')           # @SP
            self._c_command('AM', 'M-1')    # AM = M-1
            self._c_command('D', 'M')       # D = M
            self._a_command('SP')           # SP
            self._c_command('A', 'M-1')     # A = M-1
            self._c_command('M', 'M|D')     # M = M|D
        elif cmd == 'not' :
            self._a_command('SP')           # @SP
            self._c_command('A', 'M-1')     # A = M-1
            self._c_command('M', '!M')      # M = !M
        elif cmd == 'neg' :
            self._a_command('SP')           # @SP
            self._c_command('A', 'M-1')     # A = M-1
            self._c_command('M', '-M')      # M = -M
        elif cmd == 'eq'  :
            self._write_compare('JEQ')
        elif cmd == 'gt'  :
            self._write_compare('JGT')
        elif cmd == 'lt'  :
            self._write_compare('JLT')

    def write_push(self, seg, index):
        if seg == S_CONST:
            self._a_command(index)                           # Get constant
            self._c_command('D', 'A')                        # D = A
        elif seg in {S_LCL, S_ARG, S_THIS, S_THAT}:
            if index > 0:
                self._a_command(index)                       # Get index
                self._c_command('D', 'A')                    # D = A
            base_address = self._get_base_address(seg)
            self._a_command(base_address)                    # @seg[0]
            if index > 0:
                self._c_command('A', 'M+D')                  # A = seg[index]
            else:
                self._c_command('A', 'M')                    # A = seg[0] (if index = 0)
            self._c_command('D', 'M')                        # D = M
        elif seg in {S_PTR, S_TEMP}:
            base_address = self._get_base_address(seg)
            self._a_command(base_address + index)            # @seg[index]
            self._c_command('D', 'M')                        # D = M
        elif seg == S_STATIC:
            self._a_command(f"{self._vm_filename}.{index}")  # @seg[index]
            self._c_command('D', 'M')                        # D = M

        self._a_command('SP')                                # @SP
        self._c_command('A', 'M')                            # A = M
        self._c_command('M', 'D')                            # M = D
        self._a_command('SP')                                # SP
        self._c_command('M', 'M+1')                          # M = M+1

    def write_pop(self, seg, index):
        if seg in {S_LCL, S_ARG, S_THIS, S_THAT}:
            if index > 0:
                self._a_command(index)                       # Get index
                self._c_command('D', 'A')                    # D = A
            base_address = self._get_base_address(seg)
            self._a_command(base_address)                    # @seg[0]
            if index > 0:
                self._c_command('D', 'M+D')                  # D = M+D
            else:
                self._c_command('D', 'M')                    # D = M (in index = 0)
            self._a_command('R13')                           # @RAM[13] (store target address in RAM[13])
            self._c_command('M', 'D')                        # M = D
            self._a_command('SP')                            # @SP
            self._c_command('AM', 'M-1')                     # AM = M-1
            self._c_command('D', 'M')                        # D = M
            self._a_command('R13')                           # @RAM[13]
            self._c_command('A', 'M')                        # A = M
            self._c_command('M', 'D')                        # M = D
        elif seg in {S_PTR, S_TEMP}:
            self._a_command('SP')                            # @SP
            self._c_command('AM', 'M-1')                     # AM = M-1
            self._c_command('D', 'M')                        # D = M
            base_address = self._get_base_address(seg)
            self._a_command(f"R{base_address + index}")      # @seg[index]
            self._c_command('M', 'D')                        # M = D
        elif seg == S_STATIC:
            self._a_command('SP')                            # @SP
            self._c_command('AM', 'M-1')                     # AM = M-1
            self._c_command('D', 'M')                        # D = M
            self._a_command(f"{self._vm_filename}.{index}")  # @seg[index]
            self._c_command('M', 'D')                        # M = D

    def _write_compare(self, jump):
        self._a_command('SP')                       # @SP
        self._c_command('AM', 'M-1')                # AM = M-1
        self._c_command('D', 'M')                   # D = M
        self._a_command('SP')                       # @SP
        self._c_command('A', 'M-1')                 # A = M-1
        self._c_command('D', 'M-D')                 # D = M-D
        jump_label = f"LABEL_{self._label_num}"
        self._a_command(jump_label)                 # @LABEL_i
        self._c_command(None, 'D', jump=jump)       # D; jump
        self._a_command('SP')                       # @SP
        self._c_command('A', 'M-1')                 # A = M-1
        self._c_command('M', '0')                   # M = 0 (set top of stack = 0 if False)
        cont_label = f"CONT_{self._label_num}"
        self._a_command(cont_label)                 # @CONT_i
        self._c_command(None, 'D', jump='JMP')      # D; JMP
        self._l_command(jump_label)                 # (LABEL_i)
        self._a_command('SP')                       # @SP
        self._c_command('A', 'M-1')                 # A = M-1
        self._c_command('M', '-1')                  # M = -1 (set top of stack = -1 if True)
        self._l_command(cont_label)                 # (CONT_i)

        self._label_num += 1

    def write_label(self, label):
        self._l_command(label)              # (label)

    def write_if(self, label):
        self._a_command('SP')                       # @SP
        self._c_command('AM', 'M-1')                # AM = M-1
        self._c_command('D', 'M')                   # D = M
        self._a_command(f"{self._vm_filename}_{label}")                      # @label
        self._c_command(None, 'D', 'JNE')           # D; JNE (if top of stack != 0 --> True)

    def write_goto(self, label):
        self._a_command(f"{self._vm_filename}_{label}")                      # @label
        self._c_command(None, 'D', 'JMP')           # D; JMP (if command "if-goto" is False, JMP)

    def write_function(self, function, n_var):
        self._l_command(function)                   # (function)
        for _ in range(n_var):                      # Prepare n_var of local variables for calling function
            self._a_command('SP')                   # @SP
            self._c_command('A', 'M')               # A = M
            self._c_command('M', 0)                 # M = 0
            self._a_command('SP')                   # @SP
            self._c_command('M', 'M+1')             # M = M+1

    def write_return(self):
        self._a_command("LCL")                      # @LCL
        self._c_command('D', 'M')                   # D = M
        self._a_command('FrameEnd')                 # @FrameEnd (Store base address of LCL in variable "FrameEnd")
        self._c_command('M', 'D')                   # M = D
        self._a_command(5)                          # @5
        self._c_command('A', 'D-A')                 # A = D-A   (Get return address = FrameEnd - 5)
        self._c_command('D', 'M')                   # D = M
        self._a_command('ReturnAddr')               #           (Store return address in variable "ReturnAddr")
        self._c_command('M', 'D')                   # M = D
        self._a_command('SP')                       # @SP       (Pop return value to ARG)
        self._c_command('AM', 'M-1')                # AM = M-1
        self._c_command('D', 'M')                   # D = M
        self._a_command('ARG')                      # @ARG
        self._c_command('A', 'M')                   # A = M
        self._c_command('M', 'D')                   # M = D
        self._a_command('ARG')                      # @ARG      (Set SP = ARG + 1)
        self._c_command('D', 'M+1')                 # D = M+1
        self._a_command('SP')                       # @SP
        self._c_command('M', 'D')                   # M = D
        for seg in ['THAT', 'THIS', 'ARG', 'LCL']:  #           (Recover the stored address state after function return)
            self._a_command('FrameEnd')             # @FrameEnd
            self._c_command('AM', 'M-1')            # AM = M-1
            self._c_command('D', 'M')               # D = M
            self._a_command(seg)                    # @seg
            self._c_command('M', 'D')               # M = D
        self._a_command('ReturnAddr')               # @ReturnAddr
        self._c_command('A', 'M')                   # A = M
        self._c_command(None, 'D', 'JMP')           # D; JMP

    def write_call(self, function, n_arg):
        return_label = f"{function}$ret.{self._function_num}"
        self._a_command(return_label)               # @return_address
        self._c_command('D', 'A')                   # D = A
        self._a_command('SP')                       # @SP       (Store return address to stack)
        self._c_command('A', 'M')                   # A = M
        self._c_command('M', 'D')                   # M = D
        self._a_command('SP')                       # @SP
        self._c_command('M', 'M+1')                 # M = M+1
        for seg in ['LCL', 'ARG', 'THIS', 'THAT']:  #           (Store LCL, ARG, THIS, THAT to stack)
            self._a_command(seg)                    # @seg
            self._c_command('D', 'M')               # D = M
            self._a_command('SP')                   # @SP
            self._c_command('A', 'M')               # A = M
            self._c_command('M', 'D')               # M = D
            self._a_command('SP')                   # @SP
            self._c_command('M', 'M+1')             # M = M+1
        self._a_command('SP')                       # @SP       (Move pointer to "SP - 5")
        self._c_command('D', 'M')                   # D = M
        self._a_command(5)                          # @5
        self._c_command('D', 'D-A')                 # D = D-A
        if n_arg > 0:
            self._a_command(n_arg)                  # @n_arg
            self._c_command('D', 'D-A')             # D = D-A   (if n_arg > 0)
        self._a_command('ARG')                      # @ARG      (Set base address of ARG)
        self._c_command('M', 'D')                   # M = D
        self._a_command('SP')                       # @SP       (Set LCL = SP)
        self._c_command('D', 'M')                   # D = M
        self._a_command('LCL')                      # @LCL
        self._c_command('M', 'D')                   # M = D
        self.write_goto(function)                   # @function --> D; JMP
        self._l_command(return_label)               # (return_address)
        self._function_num += 1

    @staticmethod
    def _get_base_address(seg):
        base_address = {S_LCL: 'LCL', S_ARG: 'ARG', S_THIS: 'THIS', S_THAT: 'THAT', S_TEMP: 5, S_PTR: 3}
        return base_address[seg]

    def _a_command(self, address):
        self._outfile.write(f"@{address}\n")

    def _c_command(self, dest, comp, jump=None):
        if dest:
            self._outfile.write(f"{dest}=")
        self._outfile.write(f"{comp}")
        if jump:
            self._outfile.write(f";{jump}")
        self._outfile.write("\n")

    def _l_command(self, label):
        self._outfile.write(f"({label})\n")
