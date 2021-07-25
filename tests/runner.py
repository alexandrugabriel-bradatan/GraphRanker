#! /usr/bin/env python3

import sys
import os
import subprocess
import io

EXEC_PATH = sys.argv[1]
if len(sys.argv) == 3:
    TEST_DIR = os.path.join(os.environ['SOURCE_ROOT'], sys.argv[2])
else:
    TEST_DIR = os.path.join(os.environ['SOURCE_ROOT'], 'tests/open_tests')

def exists_input_file(i):
    if os.path.isfile(os.path.join(TEST_DIR, f'input_{i}')):
        return True
    else:
        return False

def check_outputs(test_out, out_file_path):
    with io.open(out_file_path, "r") as out_file:
        if out_file == None:
            exit(1)

        out_line = None
        for line in test_out.split("\n"):
            out_line = out_file.readline().rstrip("\n")

            if out_line != '' and line != '':
                out_set = set([int(i) for i in out_line.split(" ")])
                line_set = set([int(i) for i in line.split(" ")])

                if len(out_set - line_set) != 0:
                    exit(1)

def main():
    i = 1
    while exists_input_file(i):
        print(f'>> Testing input_{i}')

        out = ""
        with open(os.path.join(TEST_DIR, f'input_{i}')) as inp:
            with subprocess.Popen([EXEC_PATH], stdin=inp,
                                  stdout=subprocess.PIPE, text=True,
                                  encoding='utf8') as proc:
                out = proc.stdout.read()
                proc.wait()
                if proc.returncode != 0:
                    exit(1)
        print(f'>>>> Anlyzing output...')
        check_outputs(out.rstrip("\n"), os.path.join(TEST_DIR, f'output_{i}'))

        i += 1
    exit(0)

if __name__ == '__main__':
    main()
