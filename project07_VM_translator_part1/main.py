import sys
import glob
import Parser
import CodeWriter


def main():
    if len(sys.argv) != 2:
        print("Usage: VMtranslator [file.vm|dir]")
    else:
        infile, outfile = get_files(sys.argv[1])
        parser = Parser.Parser(infile)
        command_lines = parser.commands
        code_writer = CodeWriter.CodeWriter(command_lines, outfile)
        code_writer.write_code()
        code_writer.close_file()
        print("Translate success")


def get_files(file_or_dir):
    if file_or_dir.endswith('.vm'):
        return file_or_dir, file_or_dir.replace('.vm', '.asm')
    else:
        file_name = glob.glob(file_or_dir + '/*.vm')[0]
        return file_name, file_name.replace('.vm', '.asm')


if __name__ == '__main__':
    main()
