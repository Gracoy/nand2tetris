import re
from VMConstant import *


class Parser:
    def __init__(self):
        self._comment = re.compile('//.*$')
        self._command_type = {'add': C_ARITHMETIC, 'sub': C_ARITHMETIC, 'neg': C_ARITHMETIC,
                              'eq': C_ARITHMETIC, 'gt': C_ARITHMETIC, 'lt': C_ARITHMETIC,
                              'and': C_ARITHMETIC, 'or': C_ARITHMETIC, 'not': C_ARITHMETIC,
                              'label': C_LABEL, 'goto': C_GOTO, 'if-goto': C_IF,
                              'push': C_PUSH, 'pop': C_POP,
                              'call': C_CALL, 'return': C_RETURN, 'function': C_FUNCTION}

    def get_command_lines(self, filename):
        commands = []
        with open(filename, 'r') as f:
            for line in f:
                if self._remove_comment(line).strip():
                    new_line = line.split()
                    cmd_type = new_line[0]
                    commands.append((self._command_type[cmd_type], new_line))
        return commands

    def _remove_comment(self, line):
        return self._comment.sub('', line)


