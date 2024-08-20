"""
The following script can be used to test the RW244 SIMPL project.
See the readme for more information.

I am certainly no shell expert and used the following resource to learn how
to execute shell commands via Python:
https://janakiev.com/blog/python-shell-commands/

Author:
  Aaron Budke

Edited by: River Martin
"""
import sys
import os
import filecmp
import subprocess
import string
import glob

# Feature switches

# False -> no memory checks
# True -> memory checks
memcheck = True

# True -> prompt to save your output when
# there is not an 'expected output' file.
askToSave = True

# True -> prompt for option to see details for unknown valgrind output.
# False -> don't prompt for details of unknown valgrind output.
# (will still prompt for serious leaks)
requestDetails = True


# For colourful output.
# Source: https://stackoverflow.com/questions/287871/
#         how-to-print-colored-text-to-the-terminal
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


# Dividers
divider = "--------------------------------------------------"
small_divider = "---------------------"
other_divider = "##################################################"
green_div = bcolors.OKGREEN + divider + bcolors.ENDC
red_div = bcolors.FAIL + divider + bcolors.ENDC
blue_div = bcolors.OKBLUE + divider + bcolors.ENDC
yellow_div = green_div.replace(bcolors.OKGREEN, bcolors.WARNING)
green_hash_div = bcolors.OKGREEN + other_divider + bcolors.ENDC
yellow_hash_div = green_hash_div.replace(bcolors.OKGREEN, bcolors.WARNING)
red_hash_div = bcolors.FAIL + other_divider + bcolors.ENDC

# Directories
working_dir = os.getcwd()
input_dir = working_dir + "/input"
output_dir = working_dir + "/output"
bin_dir = "../bin"
test_bin = working_dir + "/bin"

# Commands
clean = "cd " + working_dir + "/../src && make clean"
scanner_make = clean + " && make testscanner"
parser_make = clean + " && make testparser"
typecheck_make = clean + " && make testtypechecking"
valgrind = "valgrind --leak-check=full --track-origins=yes "\
    "-s --show-leak-kinds=all "

# Messages
PASSED = bcolors.OKGREEN + "PASSED" + bcolors.ENDC
FAILED = bcolors.FAIL + "FAILED" + bcolors.ENDC
UNKNOWN = bcolors.WARNING + "UNKNOWN" + bcolors.ENDC
no_leaks = bcolors.OKGREEN + "No leaks" + bcolors.ENDC
input_prompt = "Type 'save' or 's' to save your output to an 'expected output'"\
    " file.\n(Only do this if you believe your output is correct)\n"\
    "Type anything else to delete your output file and continue testing."
no_expected_output_msg = bcolors.HEADER \
    + "Expected Output File Doesn't Exist\n"\
    + "Input File Appears Below:" + bcolors.ENDC
full_output = bcolors.OKBLUE + "To see the full valgrind output, type 'full'.\n"
response_here = "Enter your response here: "
to_continue = "Type anything else to continue, or CTRL-C to quit.\n"\
    + bcolors.ENDC
unknown_valgrind_output = bcolors.WARNING\
    + "Unknown Valgrind Output" + bcolors.ENDC
unknown_details_request = full_output\
    + "Type 'no prompt' or 'np' if you don't want to be prompted for input\n"\
    + "when the valgrind output is not recognised.\n"\
    + "Type 'stop' to stop using valgrind.\n"\
    + to_continue + response_here
leak_details_request = full_output + to_continue + response_here
ask_to_continue_memchecks = bcolors.OKBLUE \
    + "Type 'stop' to stop using valgrind.\n" + to_continue + response_here
still_reachable = bcolors.OKBLUE + \
    "Some memory is still reachable." + bcolors.ENDC
no_leaks_possible = bcolors.OKGREEN + "No leaks are possible" + bcolors.ENDC

# Helper variables
input_file = ""
output_file = ""
expected_file = ""
run_cmd = ""


def run_all():
    return
    # Run all tests.


# Shell
"""Runs the tests of the specified test suite."""


def run_tests(suite):
    make = ""
    dir_suffix = ""
    testexec = ""
    global memcheck
    global input_file
    global output_file
    global expected_file
    global run_cmd

    if suite == "scanner":
        make = scanner_make
        dir_suffix = "/scanner"
        testexec = "testscanner"
    elif suite == "parser":
        make = parser_make
        dir_suffix = "/parser"
        testexec = "simplc"
    elif suite == "typecheck":
        make = typecheck_make
        dir_suffix = "/typecheck"
        testexec = "simplc"
    else:
        print("Incorrect command line argument specified. The available options are:\n"
              + "scanner\n"
              + "parser\n"
              + "typecheck")
        return

    # Use provided Make file to create executables via shell
    os.system(make)
    print(divider)
    print("Running " + suite + " tests...")
    if suite == "parser":
        print("For parser tests, please ensure DFLAGS in your Makefile is "
              + "deactivated (commented out) before testing.")
    print(divider)
    # Create directory to house testing output if not present
    if not os.path.exists(test_bin):
        os.makedirs(test_bin)
    if not os.path.exists(test_bin + dir_suffix):
        os.makedirs(test_bin + dir_suffix)

    no_passed = 0
    no_run = 0
    num_unknown = 0
    # How to list files in a directory:
    # https://stackoverflow.com/questions/3207219/how-do-i-list-all-files-of-a-directory
    # How to list files in a directory and all subdirectories (recursively):
    # https://stackoverflow.com/questions/2186525/how-to-use-glob-to-find-files-recursively
    for f in glob.glob(input_dir + dir_suffix + '/**/*.simpl', recursive=True):
        # Files
        input_file = f
        expected_file = f.replace(input_dir, output_dir)\
            .replace(".simpl", ".txt")
        output_file = test_bin + f.replace(input_dir, "")\
            .replace(".simpl", "_output.txt")
        # Commands
        run_cmd = "../bin/{} {} ".format(testexec, input_file)
        run_test = run_cmd + ">{} 2>&1".format(output_file)
        mkdirs = "mkdir -p `dirname {}`".format(output_file)

        # Make sub-directories in 'output/' if necessary.
        # (Existing directories will not be overwritten)
        os.system(mkdirs)

        print("Testing " + input_file)

        if (not os.path.exists(expected_file)):
            run_without_expect(run_test)
            if (memcheck):
                check_memory()
                print("Test Case    : " + UNKNOWN)
            print(green_div)
            if (askToSave):
                request_save(output_file, expected_file)
            else:
                # Delete output by default
                os.system("rm " + output_file)
                print(bcolors.OKBLUE + "Output deleted." + bcolors.ENDC)
            print(green_div)
            no_run += 1
            num_unknown += 1
            continue

        expected = open(expected_file)
        output = open(output_file, 'w')
        # Run test executable and pipe to corresponding output (chains stdout and stderr).
        testrun = subprocess.run([bin_dir + "/" + testexec, input_file], stderr=subprocess.STDOUT,
                       stdout=output)
        output.close()  # To later reset read/write pointer.

        if (memcheck):
            check_memory()

        # System for comparing two files:
        # https://www.geeksforgeeks.org/how-to-compare-two-text-files-in-python/
        expected_read = expected.read().strip('\n').strip('\t')
        output = open(output_file, 'r')
        output_read = output.read().strip('\n').strip('\t')
        output.close()
        pass_status = True
        if testrun.returncode == -11:  # Special fail case for segfaults:
            # Return codes of segfaults:
            # https://stackoverflow.com/questions/14599670/what-error-code-does-a-
            # process-that-segfaults-return
            print("\x1b[0;49;91mFAILED - SEGFAULT\x1b[0m")
            print(bcolors.HEADER + "Input File Appears Below:" + bcolors.ENDC)

            print(red_div)
            os.system("cat -n {}".format(input_file))
            print(red_div)
            print("Consider running your program manually against this test case to "
                  + "better\nidentify the cause of this segfault.")
            print(red_div)
        elif not expected_read == output_read:
            pass_status = False
            print(small_divider)
            print(bcolors.OKGREEN + "Expected:\n" +
                  bcolors.ENDC + expected_read)
            print(small_divider)
            print(bcolors.FAIL + "Actual:\n" + bcolors.ENDC + output_read)
            print(small_divider)
            print("Input:")
            print(red_hash_div)
            os.system("cat -n {}".format(input_file))
            print(red_hash_div)
            input("Type anything when you are ready to continue testing:")
            print(divider)
            if (memcheck):
                print("Test Case    : " + FAILED)
            else:
                print("\x1b[0;49;91mFAILED\x1b[0m")
        else:
            no_passed += 1
            if (memcheck):
                print("Test Case    : " + PASSED)
            else:
                print("\x1b[0;49;92mPASSED\x1b[0m")
            print(divider)
            expected.close()

        no_run += 1

    # Testing coverage report:
    print("\x1b[7;49;34mCOVERAGE REPORT:\x1b[0m")
    print("Total tests run     : {}".format(no_run))
    print("Total tests passed  : {}".format(no_passed))
    print("Total tests failed  : {}".format(no_run - no_passed - num_unknown))
    print("Total tests unknown : {}".format(num_unknown))
    print(divider)

    # delete everything inside test_bin
    print("Deleting the contents of tests/bin")
    print("rm -r bin/*")
    os.system("rm -r bin/*")

    # Clean bin for subsequent reuse.
    print("Cleaning executables bin...")
    os.system(clean)


def something_reachable(summary):
    return summary.find("still reachable: 0 bytes") == -1 \
        and summary.find("no leaks are possible") == -1

def check_memory():
    global memcheck
    global requestDetails
    memcheck_file = output_file.replace("_output.txt", "_memcheck.txt")
    leak_check = valgrind + run_cmd + ">{} 2>&1".format(memcheck_file)
    os.system(leak_check)
    os.system("tail {} --lines=8 > temp.txt".format(memcheck_file))
    error_file = open("temp.txt")
    summary = error_file.read()
    error_file.close()
    output = open(output_file, 'r')
    output_text = output.read()
    output.close()
    if (summary.find("0 errors") != -1):
        if (summary.find("no leaks are possible") != -1):
            print("Memory Check : " + PASSED)
        elif (something_reachable(summary) and output_text != ""):
            print("Memory Check : " + PASSED)
        else:
            print(unknown_valgrind_output)
            print(yellow_hash_div)
            print(summary, end="")
            print(yellow_hash_div)

            print(bcolors.HEADER + "Input File Appears Below:" + bcolors.ENDC)
            print(yellow_div)
            os.system("cat -n {}".format(input_file))
            print(yellow_div)

            print(bcolors.HEADER + "Your Output:" + bcolors.ENDC)
            print(yellow_div)
            os.system("cat {}".format(output_file))
            print(yellow_div)

            if (requestDetails):
                response = input(unknown_details_request)
                if (response == "full"):
                    print(yellow_div)
                    os.system("cat {}".format(memcheck_file))
                    print(yellow_div)
                elif (response == 'np' or response == 'no prompt'):
                    requestDetails = False
                elif (response == 'stop'):
                    memcheck = False
                if (response != "full"):
                    print(divider)
            print("Memory Check : " + UNKNOWN)
        os.system("rm {}".format(memcheck_file))
    else:
        print(bcolors.FAIL + "Leaks Found" + bcolors.ENDC)
        print(red_hash_div)
        print(summary, end="")
        print(red_hash_div)
        response = input(leak_details_request)
        if (response == "full"):
            print(red_div)
            os.system("cat {}".format(memcheck_file))
            print(red_div)
        more_checks = input(ask_to_continue_memchecks)
        if (more_checks == 'stop'):
            memcheck = False
    os.system("rm temp.txt")


# Save or delete your program's output.
def request_save(output_file, expected_file):
    save_output = "mv {} {}".format(output_file, expected_file)
    print(green_hash_div)
    print(bcolors.OKBLUE + input_prompt + bcolors.ENDC)
    choice = input("Enter your choice here: ")
    if (choice == 'save' or choice == 's'):
        # Make sub-directories in 'output/' if necessary.
        # (Existing directories will not be overwritten)
        mkdirs = "mkdir -p `dirname {}`".format(expected_file)
        os.system(mkdirs)
        os.system(save_output)
        print(bcolors.OKBLUE + "Output saved." + bcolors.ENDC)
    else:
        os.system("rm " + output_file)
        print(bcolors.OKBLUE + "Output deleted." + bcolors.ENDC)
    print(green_hash_div)


def run_without_expect(run_test):
    # Run the test without expected output
    print(no_expected_output_msg)
    print(green_div)
    os.system("cat -n {}".format(input_file))
    print(green_div)
    print(bcolors.HEADER + "Your Output:" + bcolors.ENDC)
    print(green_div)
    os.system(run_test)
    os.system("cat {}".format(output_file))
    print(green_div)


if __name__ == "__main__":
    arg_len = len(sys.argv)
    if arg_len == 2:
        run_tests(sys.argv[1])
    else:
        print("Incorrect format. Expects at least one command line argument.")
        print("Incorrect command line argument specified. The available options are:\n"
              + "scanner\n"
              + "parser\n"
              + "typecheck")
