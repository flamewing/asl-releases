/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*****************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_BUFFER_SIZE 4096

static char io_buffer[32768];

#ifdef _WIN32
#    define PATHSEP "\\"
#else
#    define PATHSEP "/"
#endif

// TODO: Need to fix AS path handling so this is not needed
static char* convert_path(char* path) {
#ifdef _WIN32
    char* p = path;
    while (*p) {
        if (*p == '/') {
            *p = '\\';
        }
        p++;
    }
#endif
    return path;
}

/**
 * @brief Prints the contents of a document file.
 *
 * This function opens the test's document file (if it exists) and prints its
 * contents to stdout.
 *
 * @param file_name Buffer to use for filename.
 * @param file_size The size of the `file_name` buffer.
 * @param source_dir The test's source directory.
 * @param test_name The name of the test associated with the document file.
 */
static void print_doc(
        char* file_name, size_t file_size, char const* source_dir,
        char const* test_name) {
    snprintf(file_name, file_size, "%s" PATHSEP "%s.doc", source_dir, test_name);
    FILE* doc = fopen(file_name, "r");
    if (doc != NULL) {
        while (fgets(io_buffer, sizeof(io_buffer), doc) != NULL) {
            puts(io_buffer);
        }
        fclose(doc);
    }
}

/**
 * Reads flags for ASL from a file and stores them in a buffer.
 *
 * @param file_name Buffer to use for filename.
 * @param file_size The size of the `file_name` buffer.
 * @param source_dir The test's source directory.
 * @param buffer The buffer to store the read flags.
 * @param size The size of the buffer.
 */
static void read_flags(
        char* file_name, size_t file_size, char const* source_dir, char* buffer,
        size_t size) {
    snprintf(file_name, file_size, "%s" PATHSEP "asflags", source_dir);
    FILE* flags = fopen(file_name, "r");
    if (flags == NULL) {
        *buffer = 0;
        return;
    }
    if (fgets(buffer, (int)size, flags) == NULL) {
        *buffer = 0;
    }
    fclose(flags);
    for (char* p = buffer; *p; p++) {
        if (*p == '\n' || *p == '\r') {
            *p = 0;
            break;
        }
    }
}

/**
 * Prints a failure message for a test and logs the failure.
 *
 * @param test_name The name of the test that failed.
 */
static void test_failed(char const* test_name) {
    printf("%s : failed\n", test_name);
}

/**
 * @brief Dumps the contents of a file in hexadecimal format.
 *
 * This function takes a pointer to the data and the size of the data, and
 * prints the contents of the file in hexadecimal format. It prints 16 bytes
 * per line, with each byte separated by a space. A new line is printed after
 * every 16 bytes.
 *
 * @param data A pointer to the data to be dumped.
 * @param size The size of the data to be dumped.
 */
static void dump_file(char const* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", +(unsigned char)data[i]);
        if (i % 16 == 15) {
            printf("\n");
        }
    }
    printf("\n");
}

/**
 * Compares actual output file to expected output file to see if they match.
 *
 * @param source_dir The test's source directory.
 * @param output_dir The test's output directory.
 * @param test_name The name of the test.
 * @return Returns true if the files are identical, false otherwise.
 */
static bool compare_files(
        char const* source_dir, char const* output_dir, char const* test_name) {
    char ori_file[PATH_BUFFER_SIZE];
    snprintf(ori_file, sizeof(ori_file), "%s" PATHSEP "%s.ori", source_dir, test_name);
    FILE* ori = fopen(ori_file, "rb");
    if (ori == NULL) {
        // This should never happen...
        test_failed(test_name);
        printf("Expected file '%s' is missing\n", ori_file);
        return false;
    }

    char bin_file[PATH_BUFFER_SIZE];
    snprintf(bin_file, sizeof(bin_file), "%s" PATHSEP "%s.bin", output_dir, test_name);
    FILE* bin = fopen(bin_file, "rb");
    if (bin == NULL) {
        test_failed(test_name);
        printf("Obtained file '%s' is missing\n", bin_file);
        fclose(ori);
        return false;
    }

    fseek(ori, 0, SEEK_END);
    size_t ori_size = (size_t)ftell(ori);
    fseek(ori, 0, SEEK_SET);
    char* ori_data = (char*)malloc(ori_size);
    if (ori_data == NULL) {
        test_failed(test_name);
        printf("Failed to allocate %zu bytes for '%s'\n", ori_size, ori_file);
        fclose(ori);
        fclose(bin);
        return false;
    }

    fseek(bin, 0, SEEK_END);
    size_t bin_size = (size_t)ftell(bin);
    fseek(bin, 0, SEEK_SET);
    char* bin_data = (char*)malloc(bin_size);
    if (bin_data == NULL) {
        test_failed(test_name);
        printf("Failed to allocate %zu bytes for '%s'\n", bin_size, bin_file);
        free(ori_data);
        fclose(ori);
        fclose(bin);
        return false;
    }

    fread(ori_data, 1, ori_size, ori);
    fread(bin_data, 1, bin_size, bin);
    fclose(ori);
    fclose(bin);

    bool result;
    if (ori_size != bin_size || memcmp(ori_data, bin_data, ori_size) != 0) {
        test_failed(test_name);
        printf("Expected size: %zu\n", ori_size);
        printf("Obtained size: %zu\n", bin_size);
        printf("Expected data:\n");
        dump_file(ori_data, ori_size);
        printf("Obtained data:\n");
        dump_file(bin_data, bin_size);
        result = false;
    } else {
        printf("%s : OK\n", test_name);
        result = true;
    }

    free(ori_data);
    free(bin_data);
    return result;
}

int main(int argc, char* argv[]) {
    // argv[0] is the name of the program
    // argv[1] is verbose mode flag, 0 for off, nonzero for on
    // argv[2] is the source directory of the test
    // argv[3] is the name of the test
    // argv[4] is the include directory
    // argv[5] is the asl executable path
    // argv[6] is the p2bin executable path
    // argv[7] is the test output directory
    // In the directory specified by argv[2], are the following files:
    // - <test name>.asm (required)
    // - <test name>.ori (required)
    // - <test name>.doc (optional)
    // - asflags (optional)

    if (argc < 8) {
        printf("Usage: %s <verbose> <source_dir> <test_name> <include_dir> "
               "<asl_path> <p2bin_path> <output_dir>\n",
               argv[0]);
        bool const is_help = (argc == 2 && strcmp(argv[1], "--help") == 0);
        return is_help ? 0 : 1;
    }

    bool const  verbose     = strcmp(argv[1], "1") == 0;
    char const* quiet       = verbose == true ? "" : "-q";
    char const* source_dir  = convert_path(argv[2]);
    char const* test_name   = argv[3];
    char const* include_dir = convert_path(argv[4]);
    char const* asl_path    = convert_path(argv[5]);
    char const* p2bin_path  = convert_path(argv[6]);
    char const* output_dir  = convert_path(argv[7]);

    char file_name[PATH_BUFFER_SIZE];
    memset(file_name, 0, sizeof(file_name));

    printf("Running test %s...", test_name);
    printf("Test %s:\n", test_name);

    snprintf(file_name, sizeof(file_name), "%s.bin", test_name);
    remove(file_name);
    snprintf(file_name, sizeof(file_name), "%s.log", test_name);
    remove(file_name);
    snprintf(file_name, sizeof(file_name), "%s.h", test_name);
    remove(file_name);

    if (verbose) {
        print_doc(file_name, sizeof(file_name), source_dir, test_name);
    }

    char flags[1024];
    read_flags(file_name, sizeof(file_name), source_dir, flags, sizeof(flags));

    snprintf(
            io_buffer, sizeof(io_buffer),
            "%s %s %s -i %s %s" PATHSEP "%s.asm -o %s" PATHSEP "%s.p -shareout %s" PATHSEP
            "%s.h ",
            asl_path, flags, quiet, include_dir, source_dir, test_name, output_dir,
            test_name, output_dir, test_name);
    printf("Running: %s\n", io_buffer);
    int asl_ret = system(io_buffer);

    if (asl_ret != 0) {
        test_failed(test_name);
        printf("ASL failed with code %d\n", asl_ret);
        return 1;
    }

    snprintf(
            io_buffer, sizeof(io_buffer), "%s %s -k -l 0 -r 0x-0x %s" PATHSEP "%s",
            p2bin_path, quiet, output_dir, test_name);
    printf("Running: %s\n", io_buffer);
    int p2bin_ret = system(io_buffer);

    if (p2bin_ret != 0) {
        test_failed(test_name);
        printf("P2BIN failed with code %d\n", p2bin_ret);
        return 1;
    }

    bool passed = compare_files(source_dir, output_dir, test_name);
    return passed ? 0 : 1;
}
