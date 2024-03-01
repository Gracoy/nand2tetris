import sys
import glob
import Parser
import CodeWriter


def main():
    if len(sys.argv) != 2:
        print("Usage: main.py file.vm or dir")
    else:
        infiles, outfile = get_files(sys.argv[1])
        if not infiles:
            print(f"No *.vm file in directory: {sys.argv[1]}")
            return
        parser = Parser.Parser()
        code_writer = CodeWriter.CodeWriter(outfile)
        code_writer.sys_init()
        for infile in infiles:
            file_command = parser.get_command_lines(infile)
            code_writer.set_vm_filename(infile)
            code_writer.write_code(file_command)
        code_writer.close_file()
        code_writer.move_file()
        print(f"Translate success: {outfile}")


def get_files(file_or_dir):
    if file_or_dir.endswith('.vm'):
        return [file_or_dir], file_or_dir.replace('.vm', '.asm')
    else:
        outfile = '\\' + file_or_dir.split('\\')[-1]
        return glob.glob(file_or_dir + '\\*.vm'), file_or_dir + outfile + '.asm'


if __name__ == '__main__':
    main()
