/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"

#include "addrspace.h"
#include "bpemu.h"
#include "cmdarg.h"
#include "codechunks.h"
#include "fileformat.h"
#include "nlmessages.h"
#include "nls.h"
#include "toolutils.h"

#include <string.h>

static void put32(Byte* dest, LongWord value) {
    for (int z = 0; z < 4; z++, value >>= 8) {
        dest[z] = value & 255;
    }
}

static int catalog(char const* mode) {
    char const ident[]    = "AS Message Catalog - not readable\n\032\004";
    Byte       data[4096] = {0};
    size_t     pos        = sizeof(ident) - 1;
    size_t     length_pos, count_pos, table_pos, string_pos;
    TMsgCat    cat = {NULL, NULL, 0};

    memcpy(data, ident, pos);
    put32(data + pos, 1234);
    put32(data + pos + 4, 5678);
    pos += 8;
    memcpy(data + pos, "en", 3);
    pos += 3;
    length_pos = pos;
    put32(data + pos, 12);
    pos += 4;
    count_pos = pos;
    put32(data + pos, 0);
    pos += 4;
    table_pos = pos + 5;
    put32(data + pos, table_pos);
    pos += 4;
    data[pos++] = 0; /* End of language directory. */
    string_pos  = pos + 8;
    put32(data + pos, string_pos);
    put32(data + pos + 4, string_pos + 6);
    pos += 8;
    memcpy(data + pos, "hello\0world", 12);
    pos += 12;

    if (!strcmp(mode, "negative-length")) {
        put32(data + length_pos, 0xffffffffu);
    } else if (!strcmp(mode, "huge-length")) {
        put32(data + length_pos, 0x7fffffffu);
    } else if (!strcmp(mode, "negative-position")) {
        put32(data + count_pos + 4, 0xffffffffu);
    } else if (!strcmp(mode, "countries")) {
        put32(data + count_pos, 101);
    } else if (!strcmp(mode, "negative-countries")) {
        put32(data + count_pos, 0xffffffffu);
    } else if (!strcmp(mode, "empty-table")) {
        put32(data + table_pos, table_pos);
    } else if (!strcmp(mode, "huge-table")) {
        put32(data + table_pos, 0x7fffffffu);
    } else if (!strcmp(mode, "misaligned-table")) {
        put32(data + table_pos, string_pos - 1);
    } else if (!strcmp(mode, "negative-index")) {
        put32(data + table_pos + 4, 0xffffffffu);
    } else if (!strcmp(mode, "huge-index")) {
        put32(data + table_pos + 4, 0x7fffffffu);
    } else if (!strcmp(mode, "index-at-end")) {
        put32(data + table_pos + 4, string_pos + 12);
    } else if (!strcmp(mode, "unterminated")) {
        data[pos - 1] = 'x';
    } else if (!strcmp(mode, "long-language")) {
        memset(data + sizeof(ident) - 1 + 8, 'x', 2050);
        pos = sizeof(ident) - 1 + 8 + 2050;
    } else if (!strcmp(mode, "truncated")) {
        pos--;
    }
    msg_catalog_open_buffer(&cat, data, pos, 1234, 5678);
    if (cat.MsgCount != 2 || strcmp(catgetmessage(&cat, 0), "hello")
        || strcmp(catgetmessage(&cat, 1), "world")) {
        return 1;
    }
    free(cat.MsgBlock);
    free(cat.StrPosis);
    return 0;
}

static int relocations(void) {
    static struct {
        char const* name;
        LongWord    reloc_count, export_count, string_length, string_offset;
        size_t      file_length;
        Boolean     valid, terminated;
    } const cases[] = {
            {        "relocation only",           1,           0,           2,           0, 30,  True,  True},
            {            "export only",           0,           1,           2,           0, 30,  True,  True},
            {                  "empty",           0,           0,           0,           0, 12,  True,  True},
            {         "negative count", 0xffffffffu,           0,           0,           0, 12, False,  True},
            {  "negative export count",           0, 0xffffffffu,           0,           0, 12, False,  True},
            {             "huge count", 0x7fffffffu,           0,           2,           0, 30, False,  True},
            {      "huge export count",           0, 0x7fffffffu,           2,           0, 30, False,  True},
            {           "huge strings",           1,           0, 0xffffffffu,           0, 30, False,  True},
            {       "truncated header",           1,           0,           2,           0,  8, False,  True},
            {        "truncated entry",           1,           0,           2,           0, 20, False,  True},
            {        "missing strings",           1,           0,           2,           0, 28, False,  True},
            {      "relocation offset",           1,           0,           2,           2, 30, False,  True},
            {          "export offset",           0,           1,           2, 0xffffffffu, 30, False,  True},
            {"unterminated relocation",           1,           0,           2,           0, 30, False, False},
            {    "unterminated export",           0,           1,           2,           0, 30, False, False},
    };

    for (size_t z = 0; z < as_array_size(cases); z++) {
        Byte       data[30] = {0};
        FILE*      file     = tmpfile();
        PRelocInfo info;
        if (!file) {
            return 1;
        }
        put32(data, cases[z].reloc_count);
        put32(data + 4, cases[z].export_count);
        put32(data + 8, cases[z].string_length);
        put32(data + (cases[z].reloc_count ? 20 : 12), cases[z].string_offset);
        data[28] = 'x';
        data[29] = cases[z].terminated ? 0 : 'y';
        if (fwrite(data, 1, cases[z].file_length, file) != cases[z].file_length) {
            fclose(file);
            return 1;
        }
        rewind(file);
        info = ReadRelocInfo(file);
        fclose(file);
        if (!!info != cases[z].valid) {
            fprintf(stderr, "Incorrect relocation result: %s\n", cases[z].name);
            if (info) {
                DestroyRelocInfo(info);
            }
            return 1;
        }
        if (info) {
            if ((info->RelocCount && strcmp(info->RelocEntries[0].Name, "x"))
                || (info->ExportCount && strcmp(info->ExportEntries[0].Name, "x"))) {
                DestroyRelocInfo(info);
                return 1;
            }
            DestroyRelocInfo(info);
        }
    }
    return 0;
}

static int record(char const* mode, char const* prog_path) {
    Byte  data[] = {FileHeaderDataRec, 0x66, SegCode, 1};
    Byte  header, cpu, segment, gran;
    FILE* file = tmpfile();
    if (!file) {
        return 1;
    }
    toolutils_init(prog_path);
    if (!strcmp(mode, "zero-gran")) {
        data[3] = 0;
    } else if (!strcmp(mode, "invalid-bit-gran")) {
        data[3] = 0xfd;
    } else if (!strcmp(mode, "segment")) {
        data[2] = 255;
    } else if (!strcmp(mode, "no-segment")) {
        data[2] = SegNone;
    } else if (!strcmp(mode, "unknown-cpu")) {
        data[0] = 0x2f; /* Unassigned compact CPU identifier. */
    } else if (!strcmp(mode, "bit-gran")) {
        data[3] = FileGranularity_1;
    } else if (!strcmp(mode, "nibble-gran")) {
        data[3] = FileGranularity_4;
    }
    if (fwrite(data, 1, sizeof(data), file) != sizeof(data)) {
        fclose(file);
        return 1;
    }
    rewind(file);
    ReadRecordHeader(&header, &cpu, &segment, &gran, "test record", file);
    fclose(file);
    return header != FileHeaderDataRec || cpu != data[1] || segment != data[2]
           || gran != data[3];
}

int main(int argc, char** argv) {
    nls_init();
    if (!NLS_Initialize(&argc, argv) || argc < 2) {
        return 1;
    }
    if (!strcmp(argv[1], "relocations")) {
        return relocations();
    }
    if (!strcmp(argv[1], "small-path")) {
        char path[3] = {'x', 'x', 'x'};
        FExpand(path, sizeof(path), "..");
        return !memchr(path, '\0', sizeof(path));
    }
    if (argc < 3) {
        return 1;
    }
    if (!strcmp(argv[1], "catalog")) {
        return catalog(argv[2]);
    }
    if (!strcmp(argv[1], "record")) {
        return record(argv[2], argv[0]);
    }
    if (!strcmp(argv[1], "chunk") && argc == 4) {
        tCodeChunk chunk;
        int ret, failed;
        InitCodeChunk(&chunk);
        ret = ReadCodeChunk(&chunk, argv[2], 0, 0, 1);
        if (!strcmp(argv[3], "empty")) {
            failed = ret != 0 || chunk.Length != 0;
        } else if (!strcmp(argv[3], "data")) {
            failed = ret != 0 || chunk.Length != 4 || memcmp(chunk.pCode, "test", 4);
        } else {
            failed = ret == 0;
        }
        FreeCodeChunk(&chunk);
        return failed;
    }
    if (!strcmp(argv[1], "response") && argc == 4) {
        as_cmd_results_t results;
        char*            args[] = {argv[0], argv[2]};
        as_cmd_result_t  ret;
        int              failed;
        as_cmdarg_init(argv[0]);
        ret = as_cmd_process(2, args, "ASL_COVERITY_TEST_CMD", &results);
        if (!strcmp(argv[3], "help")) {
            failed = ret != e_cmd_ok || !results.write_help_exit;
        } else {
            failed = ret != e_cmd_err || !*results.error_arg;
        }
        ClearStringList(&results.file_arg_list);
        return failed;
    }
    return 1;
}
