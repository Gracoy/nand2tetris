from VMConstant import *


class CodeWriter:
    def __init__(self, command_lines, outf):
        self._vmfile = outf.split('\\')[-1].split('.')[0]
        self._outfile = open(outf, 'w')

        self.command_lines = command_lines
        self._label_num = 0

    def write_code(self):
        for line in self.command_lines:
            self._outfile.write(f"// {' '.join(line[1])}\n")
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

    def close_file(self):
        self._outfile.close()

    def write_arithmetic(self, cmd):
        if cmd == 'add' :
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            self._a_command('SP')
            self._c_command('A', 'M-1')
            self._c_command('M', 'M+D')
        elif cmd == 'sub' :
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            self._a_command('SP')
            self._c_command('A', 'M-1')
            self._c_command('M', 'M-D')
        elif cmd == 'and' :
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            self._a_command('SP')
            self._c_command('A', 'M-1')
            self._c_command('M', 'M&D')
        elif cmd == 'or'  :
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            self._a_command('SP')
            self._c_command('A', 'M-1')
            self._c_command('M', 'M|D')
        elif cmd == 'not' :
            self._a_command('SP')
            self._c_command('A', 'M-1')
            self._c_command('M', '!M')
        elif cmd == 'neg' :
            self._a_command('SP')
            self._c_command('A', 'M-1')
            self._c_command('M', '-M')
        elif cmd == 'eq'  :
            self._write_compare('JEQ')
        elif cmd == 'gt'  :
            self._write_compare('JGT')
        elif cmd == 'lt'  :
            self._write_compare('JLT')

    def write_push(self, seg, index):
        if seg == S_CONST:
            self._a_command(index)
            self._c_command('D', 'A')
        elif seg in {S_LCL, S_ARG, S_THIS, S_THAT}:
            if index > 0:
                self._a_command(index)
                self._c_command('D', 'A')
            base_address = self._get_base_address(seg)
            self._a_command(base_address)
            if index > 0:
                self._c_command('A', 'M+D')
            else:
                self._c_command('A', 'M')
            self._c_command('D', 'M')
        elif seg in {S_PTR, S_TEMP}:
            base_address = self._get_base_address(seg)
            self._a_command(base_address + index)
            self._c_command('D', 'M')
        elif seg == S_STATIC:
            self._a_command(f"{self._vmfile}.{index}")
            self._c_command('D', 'M')

        self._a_command('SP')
        self._c_command('A', 'M')
        self._c_command('M', 'D')
        self._a_command('SP')
        self._c_command('M', 'M+1')

    def write_pop(self, seg, index):
        if seg in {S_LCL, S_ARG, S_THIS, S_THAT}:
            if index > 0:
                self._a_command(index)
                self._c_command('D', 'A')
            base_address = self._get_base_address(seg)
            self._a_command(base_address)
            if index > 0:
                self._c_command('D', 'M+D')
            else:
                self._c_command('D', 'M')
            self._a_command('R13')
            self._c_command('M', 'D')
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            self._a_command('R13')
            self._c_command('A', 'M')
            self._c_command('M', 'D')
        elif seg in {S_PTR, S_TEMP}:
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            base_address = self._get_base_address(seg)
            self._a_command(f"R{base_address + index}")
            self._c_command('M', 'D')
        elif seg == S_STATIC:
            self._a_command('SP')
            self._c_command('AM', 'M-1')
            self._c_command('D', 'M')
            self._a_command(f"{self._vmfile}.{index}")
            self._c_command('M', 'D')

    def _write_compare(self, jump):
        self._a_command('SP')
        self._c_command('AM', 'M-1')
        self._c_command('D', 'M')
        self._a_command('SP')
        self._c_command('A', 'M-1')
        self._c_command('D', 'M-D')
        jump_label = f"LABEL_{self._label_num}"
        self._a_command(jump_label)
        self._c_command(None, 'D', jump=jump)
        self._a_command('SP')
        self._c_command('A', 'M-1')
        self._c_command('M', '0')
        cont_label = f"CONT_{self._label_num}"
        self._a_command(cont_label)
        self._c_command(None, 'D', jump='JMP')
        self._l_command(jump_label)
        self._a_command('SP')
        self._c_command('A', 'M-1')
        self._c_command('M', '-1')
        self._l_command(cont_label)

        self._label_num += 1

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
