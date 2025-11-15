/***************************************************************************
 * DurisMUD Player File Converter - Integrated Version
 *
 * Converts player files from big-endian (old format) to little-endian (new format)
 *
 * This converter handles the endianness change introduced by the testmud merge
 * (commit 2978989b) which removed htonl()/htons() calls from ADD_SHORT, ADD_INT,
 * and ADD_LONG macros in src/files.h.
 *
 * IMPORTANT: This also handles the pc_timer format change from INT to LONG,
 * which expands the status section by 40 bytes.
 *
 * Usage:
 *   pfile_converter [OPTIONS] <input_file> [output_file]
 *
 * Options:
 *   -v, --verbose    Verbose diagnostic output
 *   --dry-run        Validate input only, don't write output
 *   --backup         Create .backup file before conversion
 *   -h, --help       Show this help message
 *
 * !! I only tested this with 5 char. !!
 * Date: 15/10/2025 - Arih
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

/***************************************************************************
 * Constants and Defines
 ***************************************************************************/

/* File format version constants */
#define SAV_SAVEVERS    5     /* Overall save format version */
#define SAV_STATVERS    47    /* Status section version */
#define SAV_SKILLVERS   2     /* Skills section version */
#define SAV_WTNSVERS    2     /* Witness section version */
#define SAV_AFFVERS     7     /* Affects section version */
#define SAV_ITEMVERS    35    /* Items section version */

/* Size constants from source code */
#define MAX_CIRCLE          12
#define MAX_TONGUE          29
#define MAX_INTRO           150
#define MAX_FORGE_ITEMS     1000
#define NUMB_PC_TIMERS      10
#define MAX_COND            5
#define MAX_SKILLS          2000
#define MAX_OBJ_AFFECT      4

/* Item flags (from src/files.h) */
#define O_F_WORN        1    /* item was equipped when saved */
#define O_F_CONTAINS    2    /* item was a container that had something in it */
#define O_F_UNIQUE      4    /* item was 'non-stock' in some way */
#define O_F_COUNT       8    /* more than 1 item of this type */
#define O_F_EOL         16   /* marks end of a 'contents' list */
#define O_F_AFFECTS     32   /* object had a decay event when saved */
#define O_F_SPELLBOOK   64   /* has spellbook array */

/* Unique field flags - BIT_N = 2^(N-1) */
#define O_U_KEYS        (1U)          /* BIT_1 */
#define O_U_DESC1       (2U)          /* BIT_2 */
#define O_U_DESC2       (4U)          /* BIT_3 */
#define O_U_DESC3       (8U)          /* BIT_4 */
#define O_U_VAL0        (16U)         /* BIT_5 */
#define O_U_VAL1        (32U)         /* BIT_6 */
#define O_U_VAL2        (64U)         /* BIT_7 */
#define O_U_VAL3        (128U)        /* BIT_8 */
#define O_U_TYPE        (256U)        /* BIT_9 */
#define O_U_WEAR        (512U)        /* BIT_10 */
#define O_U_EXTRA       (1024U)       /* BIT_11 */
#define O_U_WEIGHT      (2048U)       /* BIT_12 */
#define O_U_COST        (4096U)       /* BIT_13 */
#define O_U_BV1         (8192U)       /* BIT_14 */
#define O_U_BV2         (16384U)      /* BIT_15 */
#define O_U_AFFS        (32768U)      /* BIT_16 */
#define O_U_TRAP        (65536U)      /* BIT_17 */
#define O_U_COND        (131072U)     /* BIT_18 */
#define O_U_ANTI        (262144U)     /* BIT_19 */
#define O_U_EXTRA2      (524288U)     /* BIT_20 */
#define O_U_TIMER       (1048576U)    /* BIT_21 */
#define O_U_ANTI2       (2097152U)    /* BIT_22 */
#define O_U_EDESC       (4194304U)    /* BIT_23 */
#define O_U_MATERIAL    (8388608U)    /* BIT_24 */
#define O_U_SPACE       (16777216U)   /* BIT_25 */
#define O_U_VAL4        (33554432U)   /* BIT_26 */
#define O_U_VAL5        (67108864U)   /* BIT_27 */
#define O_U_VAL6        (134217728U)  /* BIT_28 */
#define O_U_VAL7        (268435456U)  /* BIT_29 */
#define O_U_BV3         (536870912U)  /* BIT_30 */
#define O_U_BV4         (1073741824U) /* BIT_31 */
#define O_U_BV5         (2147483648U) /* BIT_32 */

/* Maximum file size */
#define SAV_MAXSIZE     (240 * 1024)  /* 240 KB */
#define MAX_RECURSION   50            /* Max item container depth */

/***************************************************************************
 * Global Variables
 ***************************************************************************/

static int verbose_mode = 0;
static int dry_run_mode = 0;
static int backup_mode = 0;

/* Statistics */
static struct {
    int status_bytes_read;
    int status_bytes_written;
    int skills_converted;
    int witness_records;
    int affects_converted;
    int items_converted;
} stats;

/***************************************************************************
 * File Header Structure
 ***************************************************************************/

typedef struct {
    uint8_t  save_version;
    uint8_t  short_size;
    uint8_t  int_size;
    uint8_t  long_size;
    uint8_t  rent_type;
    uint32_t skill_offset;
    uint32_t witness_offset;
    uint32_t affect_offset;
    uint32_t item_offset;
    uint32_t size_offset;
    uint32_t act3_surname;
    uint32_t start_room;
    uint64_t save_time;
} pfile_header_t;

/***************************************************************************
 * Endian Conversion Functions
 ***************************************************************************/

/* Read big-endian 16-bit value and advance pointer */
static uint16_t read_be16(unsigned char **buf) {
    uint16_t val = ((uint16_t)(*buf)[0] << 8) | (uint16_t)(*buf)[1];
    *buf += 2;
    return val;
}

/* Read big-endian 32-bit value and advance pointer */
static uint32_t read_be32(unsigned char **buf) {
    uint32_t val = ((uint32_t)(*buf)[0] << 24) |
                   ((uint32_t)(*buf)[1] << 16) |
                   ((uint32_t)(*buf)[2] << 8) |
                   (uint32_t)(*buf)[3];
    *buf += 4;
    return val;
}

/* Read big-endian 64-bit value and advance pointer
 *
 * CRITICAL FIX: Old ADD_LONG macro used htonl() which is 32-bit only!
 * Even when long_size=8, old files only have 4 bytes of real data (from htonl),
 * followed by 4 bytes of padding/garbage.
 *
 * So we ALWAYS read just the first 4 bytes as the actual value.
 */
static uint64_t read_be64(unsigned char **buf, int size) {
    uint64_t val;
    if (size == 8) {
        /* Old ADD_LONG used htonl() (32-bit), so only first 4 bytes are valid */
        val = read_be32(buf);  /* Read the 4-byte big-endian value */
        *buf += 4;              /* Skip the 4 bytes of padding/garbage */
    } else {
        /* 32-bit LONG - read 4 bytes */
        val = read_be32(buf);
    }
    return val;
}

/* Write little-endian 16-bit value and advance pointer */
static void write_le16(unsigned char **buf, uint16_t val) {
    (*buf)[0] = val & 0xFF;
    (*buf)[1] = (val >> 8) & 0xFF;
    *buf += 2;
}

/* Write little-endian 32-bit value and advance pointer */
static void write_le32(unsigned char **buf, uint32_t val) {
    (*buf)[0] = val & 0xFF;
    (*buf)[1] = (val >> 8) & 0xFF;
    (*buf)[2] = (val >> 16) & 0xFF;
    (*buf)[3] = (val >> 24) & 0xFF;
    *buf += 4;
}

/* Write little-endian 64-bit value and advance pointer */
static void write_le64(unsigned char **buf, uint64_t val) {
    (*buf)[0] = val & 0xFF;
    (*buf)[1] = (val >> 8) & 0xFF;
    (*buf)[2] = (val >> 16) & 0xFF;
    (*buf)[3] = (val >> 24) & 0xFF;
    (*buf)[4] = (val >> 32) & 0xFF;
    (*buf)[5] = (val >> 40) & 0xFF;
    (*buf)[6] = (val >> 48) & 0xFF;
    (*buf)[7] = (val >> 56) & 0xFF;
    *buf += 8;
}

/***************************************************************************
 * String Conversion Functions
 ***************************************************************************/

/* Read string: big-endian length prefix + string data */
static char* read_string(unsigned char **buf) {
    uint16_t len = read_be16(buf);
    if (len == 0) {
        return NULL;
    }
    char *str = (char*)malloc(len + 1);
    if (!str) {
        fprintf(stderr, "ERROR: Failed to allocate memory for string\n");
        exit(1);
    }
    memcpy(str, *buf, len);
    str[len] = '\0';
    *buf += len;
    return str;
}

/* Write string: little-endian length prefix + string data */
static void write_string(unsigned char **out, const char *str) {
    if (!str) {
        write_le16(out, 0);
        return;
    }
    uint16_t len = strlen(str);
    write_le16(out, len);
    memcpy(*out, str, len);
    *out += len;
}

/***************************************************************************
 * Header Parsing and Conversion
 ***************************************************************************/

static int parse_header(unsigned char *buf, size_t file_size, pfile_header_t *header) {
    unsigned char *ptr = buf;

    /* Parse fixed header fields */
    header->save_version = *ptr++;
    header->short_size = *ptr++;
    header->int_size = *ptr++;
    header->long_size = *ptr++;
    header->rent_type = *ptr++;

    /* Parse big-endian offsets */
    header->skill_offset = read_be32(&ptr);
    header->witness_offset = read_be32(&ptr);
    header->affect_offset = read_be32(&ptr);
    header->item_offset = read_be32(&ptr);
    header->size_offset = read_be32(&ptr);
    header->act3_surname = read_be32(&ptr);
    header->start_room = read_be32(&ptr);
    header->save_time = read_be64(&ptr, header->long_size);

    /* Validate header */
    if (header->save_version != SAV_SAVEVERS) {
        fprintf(stderr, "ERROR: Invalid save version %d (expected %d)\n",
                header->save_version, SAV_SAVEVERS);
        return 0;
    }

    if (header->short_size != 2 || header->int_size != 4 ||
        (header->long_size != 4 && header->long_size != 8)) {
        fprintf(stderr, "ERROR: Invalid type sizes (short=%d, int=%d, long=%d)\n",
                header->short_size, header->int_size, header->long_size);
        return 0;
    }

    /* Validate offsets are within file */
    if (header->skill_offset >= file_size || header->witness_offset >= file_size ||
        header->affect_offset >= file_size || header->item_offset >= file_size ||
        header->size_offset > file_size) {
        fprintf(stderr, "ERROR: Invalid section offsets\n");
        return 0;
    }

    /* Verify section version bytes */
    if (buf[41] != SAV_STATVERS) {
        fprintf(stderr, "ERROR: Invalid status version %d at offset 41\n", buf[41]);
        return 0;
    }
    if (buf[header->skill_offset] != SAV_SKILLVERS) {
        fprintf(stderr, "ERROR: Invalid skill version at offset %u\n", header->skill_offset);
        return 0;
    }
    if (buf[header->witness_offset] != SAV_WTNSVERS) {
        fprintf(stderr, "ERROR: Invalid witness version at offset %u\n", header->witness_offset);
        return 0;
    }
    if (buf[header->affect_offset] != SAV_AFFVERS) {
        fprintf(stderr, "ERROR: Invalid affect version at offset %u\n", header->affect_offset);
        return 0;
    }
    if (buf[header->item_offset] != SAV_ITEMVERS) {
        fprintf(stderr, "ERROR: Invalid item version at offset %u\n", header->item_offset);
        return 0;
    }

    return 1;
}

static void write_header(unsigned char **out, const pfile_header_t *header) {
    /* Write fixed header fields */
    *(*out)++ = header->save_version;
    *(*out)++ = header->short_size;
    *(*out)++ = header->int_size;
    *(*out)++ = header->long_size;
    *(*out)++ = header->rent_type;

    /* Write little-endian offsets (these will be updated after conversion) */
    write_le32(out, header->skill_offset);
    write_le32(out, header->witness_offset);
    write_le32(out, header->affect_offset);
    write_le32(out, header->item_offset);
    write_le32(out, header->size_offset);
    write_le32(out, header->act3_surname);
    write_le32(out, header->start_room);
    write_le64(out, header->save_time);
}

static void print_header(const pfile_header_t *header) {
    if (!verbose_mode) return;

    printf("\n=== File Header ===\n");
    printf("Save version: %d\n", header->save_version);
    printf("Type sizes: short=%d, int=%d, long=%d\n",
           header->short_size, header->int_size, header->long_size);
    printf("Rent type: %d\n", header->rent_type);
    printf("Section offsets:\n");
    printf("  Status:  41 (implicit)\n");
    printf("  Skills:  %u\n", header->skill_offset);
    printf("  Witness: %u\n", header->witness_offset);
    printf("  Affects: %u\n", header->affect_offset);
    printf("  Items:   %u\n", header->item_offset);
    printf("  EOF:     %u\n", header->size_offset);

    /* Handle timestamp quirk */
    uint64_t timestamp = header->save_time;
    if (header->long_size == 8 && timestamp > 0x100000000ULL) {
        timestamp = timestamp >> 32;  /* Use high 32 bits */
    }
    time_t t = (time_t)timestamp;
    printf("Save time: %s", ctime(&t));
}

/***************************************************************************
 * Forward Declarations
 ***************************************************************************/

static int convert_status_section(unsigned char **in, unsigned char **out,
                                   const pfile_header_t *header);
static int convert_skills_section(unsigned char **in, unsigned char **out,
                                   const pfile_header_t *header);
static int convert_witness_section(unsigned char **in, unsigned char **out,
                                    const pfile_header_t *header);
static int convert_affects_section(unsigned char **in, unsigned char **out,
                                    const pfile_header_t *header);
static int convert_items_section(unsigned char **in, unsigned char **out,
                                  const pfile_header_t *header);

/***************************************************************************
 * Main Conversion Function
 ***************************************************************************/

static int convert_pfile(unsigned char *input_buf, size_t input_size,
                         unsigned char **output_buf, size_t *output_size) {
    pfile_header_t header;
    unsigned char *in, *out, *out_start;
    extern unsigned char *input_end;

    /* Set input buffer boundary */
    input_end = input_buf + input_size;

    /* Parse and validate header */
    if (!parse_header(input_buf, input_size, &header)) {
        return 0;
    }

    print_header(&header);

    /* Allocate output buffer (input size + 40 bytes for pc_timer expansion) */
    size_t max_output_size = input_size + 100;  /* Extra padding for safety */
    *output_buf = (unsigned char*)malloc(max_output_size);
    if (!*output_buf) {
        fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
        return 0;
    }

    out_start = *output_buf;
    out = out_start;

    /* Write header (we'll update offsets later) */
    write_header(&out, &header);

    /* Convert status section */
    in = input_buf + 41;  /* Status starts after header */
    if (!convert_status_section(&in, &out, &header)) {
        free(*output_buf);
        return 0;
    }

    /* Update skill_offset in header */
    uint32_t new_skill_offset = out - out_start;

    /* Convert skills section */
    in = input_buf + header.skill_offset;
    if (!convert_skills_section(&in, &out, &header)) {
        free(*output_buf);
        return 0;
    }

    /* Update witness_offset in header */
    uint32_t new_witness_offset = out - out_start;

    /* Convert witness section */
    in = input_buf + header.witness_offset;
    if (!convert_witness_section(&in, &out, &header)) {
        free(*output_buf);
        return 0;
    }

    /* Update affect_offset in header */
    uint32_t new_affect_offset = out - out_start;

    /* Convert affects section */
    in = input_buf + header.affect_offset;
    if (!convert_affects_section(&in, &out, &header)) {
        free(*output_buf);
        return 0;
    }

    /* Update item_offset in header */
    uint32_t new_item_offset = out - out_start;

    /* Convert items section */
    in = input_buf + header.item_offset;
    if (!convert_items_section(&in, &out, &header)) {
        free(*output_buf);
        return 0;
    }

    /* Update size_offset in header */
    uint32_t new_size_offset = out - out_start;

    /* Update all offsets in the header */
    unsigned char *header_ptr = out_start + 5;  /* Skip to first offset */
    write_le32(&header_ptr, new_skill_offset);
    write_le32(&header_ptr, new_witness_offset);
    write_le32(&header_ptr, new_affect_offset);
    write_le32(&header_ptr, new_item_offset);
    write_le32(&header_ptr, new_size_offset);

    *output_size = new_size_offset;

    if (verbose_mode) {
        printf("\n=== Conversion Summary ===\n");
        printf("Input size:  %zu bytes\n", input_size);
        printf("Output size: %zu bytes\n", *output_size);
        printf("Size change: %+zd bytes\n", (ssize_t)(*output_size - input_size));
        printf("\n=== Section Offsets ===\n");
        printf("Status:  41\n");
        printf("Skills:  %u (was %u)\n", new_skill_offset, header.skill_offset);
        printf("Witness: %u (was %u)\n", new_witness_offset, header.witness_offset);
        printf("Affects: %u (was %u)\n", new_affect_offset, header.affect_offset);
        printf("Items:   %u (was %u)\n", new_item_offset, header.item_offset);
        printf("EOF:     %u (was %u)\n", new_size_offset, header.size_offset);
    }

    return 1;
}

/***************************************************************************
 * Status Section Conversion
 ***************************************************************************/

static int convert_status_section(unsigned char **in, unsigned char **out,
                                   const pfile_header_t *header) {
    unsigned char *in_start = *in;
    unsigned char *out_start = *out;
    uint8_t version;
    char *str;
    int i;

    if (verbose_mode) {
        printf("\n=== Converting Status Section ===\n");
    }

    /* Version byte */
    version = *(*in)++;
    if (version != SAV_STATVERS) {
        fprintf(stderr, "ERROR: Invalid status version %d\n", version);
        return 0;
    }
    *(*out)++ = version;

    /* Player name (STRING) */
    str = read_string(in);
    write_string(out, str);
    if (verbose_mode && str) {
        printf("Player name: %s\n", str);
    }
    free(str);

    /* pid (INT) */
    uint32_t pid = read_be32(in);
    write_le32(out, pid);

    /* screen_length (BYTE) */
    *(*out)++ = *(*in)++;

    /* Strings: password, short_descr, long_descr, description, title */
    for (i = 0; i < 5; i++) {
        str = read_string(in);
        write_string(out, str);
        free(str);
    }

    /* m_class, secondary_class (INT, INT) */
    uint32_t m_class = read_be32(in);
    uint32_t secondary_class = read_be32(in);
    write_le32(out, m_class);
    write_le32(out, secondary_class);

    /* spec, race, racewar, level, sex (5 BYTEs) */
    for (i = 0; i < 5; i++) {
        *(*out)++ = *(*in)++;
    }

    if (verbose_mode) {
        printf("Class: %u, Level: %u, Race: %u\n", m_class, (*in)[-2], (*in)[-4]);
    }

    /* weight, height (SHORT, SHORT) */
    uint16_t weight = read_be16(in);
    uint16_t height = read_be16(in);
    write_le16(out, weight);
    write_le16(out, height);

    /* size (BYTE) */
    *(*out)++ = *(*in)++;

    /* home, birthplace, orig_birthplace (INT, INT, INT) */
    for (i = 0; i < 3; i++) {
        uint32_t val = read_be32(in);
        write_le32(out, val);
    }

    /* birth_time (LONG) */
    uint64_t birth_time = read_be64(in, header->long_size);
    write_le64(out, birth_time);

    /* played_time (INT) */
    uint32_t played_time = read_be32(in);
    write_le32(out, played_time);

    /* saved_time (LONG) */
    uint64_t saved_time = read_be64(in, header->long_size);
    write_le64(out, saved_time);

    /* perm_aging (SHORT) */
    uint16_t perm_aging = read_be16(in);
    write_le16(out, perm_aging);

    /* undead_spell_slots (BYTE[13]) */
    for (i = 0; i < MAX_CIRCLE + 1; i++) {
        *(*out)++ = *(*in)++;
    }

    /* last_level (INT, always 0) */
    uint32_t last_level = read_be32(in);
    write_le32(out, last_level);

    /* CRITICAL: pc_timer array - read as INT (old format), write as LONG (new format) */
    /* This is where the 40-byte expansion occurs: 10 timers × 4 bytes growth = 40 bytes */
    for (i = 0; i < NUMB_PC_TIMERS; i++) {
        uint32_t timer_val = read_be32(in);  /* Read as INT (4 bytes) */
        write_le64(out, (uint64_t)timer_val); /* Write as LONG (8 bytes) */
    }

    /* tongue_count (SHORT) */
    uint16_t tongue_count = read_be16(in);
    write_le16(out, tongue_count);

    /* languages (BYTE[29]) */
    for (i = 0; i < MAX_TONGUE; i++) {
        *(*out)++ = *(*in)++;
    }

    /* intro_count (SHORT) */
    uint16_t intro_count = read_be16(in);
    write_le16(out, intro_count);

    /* introd_list and introd_times arrays */
    for (i = 0; i < MAX_INTRO; i++) {
        uint32_t introd_id = read_be32(in);
        uint64_t introd_time = read_be64(in, header->long_size);
        write_le32(out, introd_id);
        write_le64(out, introd_time);
    }

    /* learned_forged_list (INT[1000]) */
    for (i = 0; i < MAX_FORGE_ITEMS; i++) {
        uint32_t forged_item = read_be32(in);
        write_le32(out, forged_item);
    }

    /* Base stats (BYTE[10]) */
    for (i = 0; i < 10; i++) {
        *(*out)++ = *(*in)++;
    }

    /* mana, base_mana, damage_taken (SHORT, SHORT, SHORT) */
    for (i = 0; i < 3; i++) {
        uint16_t val = read_be16(in);
        write_le16(out, val);
    }

    /* spells_memmed (BYTE) */
    *(*out)++ = *(*in)++;

    /* base_hit, vitality, base_vitality (SHORT, SHORT, SHORT) */
    for (i = 0; i < 3; i++) {
        uint16_t val = read_be16(in);
        write_le16(out, val);
    }

    /* Money: copper, silver, gold, platinum (INT, INT, INT, INT) */
    /* experience, padding, epics, epic_skill_points, skillpoints (INT × 5) */
    /* spell_bind_used, act, act2, vote, alignment, padding (INT × 6) */
    for (i = 0; i < 15; i++) {
        uint32_t val = read_be32(in);
        write_le32(out, val);
    }

    /* prestige, guild_id (SHORT, SHORT) */
    for (i = 0; i < 2; i++) {
        uint16_t val = read_be16(in);
        write_le16(out, val);
    }

    /* guild_status (INT) */
    uint32_t guild_status = read_be32(in);
    write_le32(out, guild_status);

    /* time_left_guild (LONG) */
    uint64_t time_left_guild = read_be64(in, header->long_size);
    write_le64(out, time_left_guild);

    /* nb_left_guild (BYTE) */
    *(*out)++ = *(*in)++;

    /* time_unspecced, frags, oldfrags (LONG, LONG, LONG) */
    for (i = 0; i < 3; i++) {
        uint64_t val = read_be64(in, header->long_size);
        write_le64(out, val);
    }

    /* numb_gcmd (INT) */
    uint32_t numb_gcmd = read_be32(in);
    write_le32(out, numb_gcmd);

    /* gcmd_arr (INT[numb_gcmd]) */
    for (i = 0; i < (int)numb_gcmd; i++) {
        uint32_t gcmd = read_be32(in);
        write_le32(out, gcmd);
    }

    /* conditions (BYTE[5]) */
    for (i = 0; i < MAX_COND; i++) {
        *(*out)++ = *(*in)++;
    }

    /* Poof strings (4 STRINGs) */
    for (i = 0; i < 4; i++) {
        str = read_string(in);
        write_string(out, str);
        free(str);
    }

    /* echo_toggle (BYTE) */
    *(*out)++ = *(*in)++;

    /* prompt (SHORT) */
    uint16_t prompt = read_be16(in);
    write_le16(out, prompt);

    /* wiz_invis, law_flags (LONG, LONG) */
    for (i = 0; i < 2; i++) {
        uint64_t val = read_be64(in, header->long_size);
        write_le64(out, val);
    }

    /* wimpy, aggressive (SHORT, SHORT) */
    for (i = 0; i < 2; i++) {
        uint16_t val = read_be16(in);
        write_le16(out, val);
    }

    /* highest_level (BYTE) */
    *(*out)++ = *(*in)++;

    /* Bank balance: copper, silver, gold, platinum (INT × 4) */
    for (i = 0; i < 4; i++) {
        uint32_t val = read_be32(in);
        write_le32(out, val);
    }

    /* numb_deaths (LONG) */
    uint64_t numb_deaths = read_be64(in, header->long_size);
    write_le64(out, numb_deaths);

    /* Quest data (14 INT fields) */
    for (i = 0; i < 14; i++) {
        uint32_t val = read_be32(in);
        write_le32(out, val);
    }

    stats.status_bytes_read = *in - in_start;
    stats.status_bytes_written = *out - out_start;

    if (verbose_mode) {
        printf("Status section: %d bytes read, %d bytes written (+%d expansion)\n",
               stats.status_bytes_read, stats.status_bytes_written,
               stats.status_bytes_written - stats.status_bytes_read);
    }

    return 1;
}

/***************************************************************************
 * Skills Section Conversion
 ***************************************************************************/

static int convert_skills_section(unsigned char **in, unsigned char **out,
                                   const pfile_header_t *header) {
    uint8_t version;
    int i;

    if (verbose_mode) {
        printf("\n=== Converting Skills Section ===\n");
    }

    /* Version byte */
    version = *(*in)++;
    if (version != SAV_SKILLVERS) {
        fprintf(stderr, "ERROR: Invalid skills version %d\n", version);
        return 0;
    }
    *(*out)++ = version;

    /* num_skills (INT) */
    uint32_t num_skills = read_be32(in);
    write_le32(out, num_skills);

    if (num_skills != MAX_SKILLS) {
        fprintf(stderr, "WARNING: num_skills=%u (expected %d)\n", num_skills, MAX_SKILLS);
    }

    /* Skill data (learned, taught, padding) × num_skills */
    for (i = 0; i < (int)num_skills; i++) {
        *(*out)++ = *(*in)++;  /* learned */
        *(*out)++ = *(*in)++;  /* taught */
        *(*out)++ = *(*in)++;  /* padding */
    }

    /* Padding INT and SHORT */
    uint32_t pad_int = read_be32(in);
    uint16_t pad_short = read_be16(in);
    write_le32(out, pad_int);
    write_le16(out, pad_short);

    stats.skills_converted = num_skills;

    if (verbose_mode) {
        printf("Skills converted: %u\n", num_skills);
    }

    return 1;
}

/***************************************************************************
 * Witness Section Conversion
 ***************************************************************************/

static int convert_witness_section(unsigned char **in, unsigned char **out,
                                    const pfile_header_t *header) {
    uint8_t version;
    char *str;
    int i;

    if (verbose_mode) {
        printf("\n=== Converting Witness Section ===\n");
    }

    /* Version byte */
    version = *(*in)++;
    if (version != SAV_WTNSVERS) {
        fprintf(stderr, "ERROR: Invalid witness version %d\n", version);
        return 0;
    }
    *(*out)++ = version;

    /* count (INT) */
    uint32_t count = read_be32(in);
    write_le32(out, count);

    /* Witness records */
    for (i = 0; i < (int)count; i++) {
        /* attacker (STRING) */
        str = read_string(in);
        write_string(out, str);
        free(str);

        /* victim (STRING) */
        str = read_string(in);
        write_string(out, str);
        free(str);

        /* time (LONG) */
        uint64_t time = read_be64(in, header->long_size);
        write_le64(out, time);

        /* crime, room (INT, INT) */
        uint32_t crime = read_be32(in);
        uint32_t room = read_be32(in);
        write_le32(out, crime);
        write_le32(out, room);
    }

    stats.witness_records = count;

    if (verbose_mode) {
        printf("Witness records converted: %u\n", count);
    }

    return 1;
}

/***************************************************************************
 * Affects Section Conversion
 ***************************************************************************/

static int convert_affects_section(unsigned char **in, unsigned char **out,
                                    const pfile_header_t *header) {
    uint8_t version;
    char *str;
    int i, j;

    if (verbose_mode) {
        printf("\n=== Converting Affects Section ===\n");
    }

    /* Version byte */
    version = *(*in)++;
    if (version != SAV_AFFVERS) {
        fprintf(stderr, "ERROR: Invalid affects version %d\n", version);
        return 0;
    }
    *(*out)++ = version;

    /* count (SHORT) */
    uint16_t count = read_be16(in);
    write_le16(out, count);

    /* Affect records */
    for (i = 0; i < count; i++) {
        /* custom_messages (BYTE) */
        uint8_t custom_messages = *(*in)++;
        *(*out)++ = custom_messages;

        /* Conditional wear-off strings */
        if (custom_messages & 1) {
            str = read_string(in);
            write_string(out, str);
            free(str);
        }
        if (custom_messages & 2) {
            str = read_string(in);
            write_string(out, str);
            free(str);
        }

        /* type (SHORT) */
        uint16_t type = read_be16(in);
        write_le16(out, type);

        /* duration (INT) */
        uint32_t duration = read_be32(in);
        write_le32(out, duration);

        /* flags (SHORT) */
        uint16_t flags = read_be16(in);
        write_le16(out, flags);

        /* modifier (INT) */
        uint32_t modifier = read_be32(in);
        write_le32(out, modifier);

        /* location (BYTE) */
        *(*out)++ = *(*in)++;

        /* 6 bitvectors (5 data + 1 padding) */
        for (j = 0; j < 6; j++) {
            uint64_t bv = read_be64(in, header->long_size);
            write_le64(out, bv);
        }
    }

    stats.affects_converted = count;

    if (verbose_mode) {
        printf("Affects converted: %u\n", count);
    }

    return 1;
}

/***************************************************************************
 * Items Section Conversion
 ***************************************************************************/

static int convert_item(unsigned char **in, unsigned char **out,
                        const pfile_header_t *header, int depth);

static int convert_items_section(unsigned char **in, unsigned char **out,
                                  const pfile_header_t *header) {
    uint8_t version;
    extern unsigned char *input_end;  /* Set by convert_pfile */

    if (verbose_mode) {
        printf("\n=== Converting Items Section ===\n");
    }

    /* Version byte */
    version = *(*in)++;
    if (version != SAV_ITEMVERS) {
        fprintf(stderr, "ERROR: Invalid items version %d\n", version);
        return 0;
    }
    *(*out)++ = version;

    /* total_item_count (INT) */
    uint32_t total_count = read_be32(in);
    write_le32(out, total_count);

    if (verbose_mode) {
        printf("Total items: %u\n", total_count);
    }

    /* Convert items recursively */
    int item_count = 0;
    uint8_t peek_flag;

    while (1) {
        /* Peek at next flag before converting */
        peek_flag = **in;

        if (!convert_item(in, out, header, 0)) {
            return 0;
        }

        /* If we just processed an EOL at top level, we're done */
        if (peek_flag == O_F_EOL) {
            break;  /* End of top-level items */
        }

        item_count++;
    }

    stats.items_converted = item_count;

    if (verbose_mode) {
        printf("Items converted: %d\n", item_count);
    }

    return 1;
}

unsigned char *input_end = NULL;  /* Buffer boundary check */

static int convert_item(unsigned char **in, unsigned char **out,
                        const pfile_header_t *header, int depth) {
    char *str;
    int i;

    if (depth > MAX_RECURSION) {
        fprintf(stderr, "ERROR: Maximum recursion depth exceeded\n");
        return 0;
    }

    /* Check if we have enough data left */
    if (*in >= input_end) {
        fprintf(stderr, "ERROR: Unexpected end of input at depth %d\n", depth);
        return 0;
    }

    /* o_f_flag (BYTE) */
    uint8_t o_f_flag = *(*in)++;
    *(*out)++ = o_f_flag;

    if (verbose_mode && depth < 2) {
        fprintf(stderr, "DEBUG: depth=%d, o_f_flag=0x%02x\n", depth, o_f_flag);
    }

    if (o_f_flag == O_F_EOL) {
        return 1;  /* End marker */
    }

    /* vnum, craftsmanship, condition (INT, SHORT, SHORT) */
    uint32_t vnum = read_be32(in);
    uint16_t craftsmanship = read_be16(in);
    uint16_t condition = read_be16(in);
    write_le32(out, vnum);
    write_le16(out, craftsmanship);
    write_le16(out, condition);

    /* Optional: worn_location (BYTE) */
    if (o_f_flag & O_F_WORN) {
        *(*out)++ = *(*in)++;
    }

    /* Optional: count (SHORT) */
    if (o_f_flag & O_F_COUNT) {
        uint16_t count = read_be16(in);
        write_le16(out, count);
    }

    /* Optional: decay affects */
    if (o_f_flag & O_F_AFFECTS) {
        uint8_t affect_count = *(*in)++;
        *(*out)++ = affect_count;

        for (i = 0; i < affect_count; i++) {
            uint32_t time = read_be32(in);
            uint16_t type = read_be16(in);
            uint16_t data = read_be16(in);
            uint32_t extra2 = read_be32(in);
            write_le32(out, time);
            write_le16(out, type);
            write_le16(out, data);
            write_le32(out, extra2);
        }
    }

    /* Optional: unique fields */
    if (o_f_flag & O_F_UNIQUE) {
        uint32_t o_u_flag = read_be32(in);
        write_le32(out, o_u_flag);

        if (o_u_flag & O_U_KEYS) {
            str = read_string(in);
            write_string(out, str);
            free(str);
        }
        if (o_u_flag & O_U_DESC1) {
            str = read_string(in);
            write_string(out, str);
            free(str);
        }
        if (o_u_flag & O_U_DESC2) {
            str = read_string(in);
            write_string(out, str);
            free(str);
        }
        if (o_u_flag & O_U_DESC3) {
            str = read_string(in);
            write_string(out, str);
            free(str);
        }
        if (o_u_flag & O_U_EDESC) {
            uint16_t edesc_count = read_be16(in);
            write_le16(out, edesc_count);
            for (i = 0; i < edesc_count; i++) {
                str = read_string(in);
                write_string(out, str);
                free(str);
                str = read_string(in);
                write_string(out, str);
                free(str);
            }
        }

        /* value[0-7] */
        if (o_u_flag & O_U_VAL0) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL1) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL2) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL3) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL4) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL5) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL6) { uint32_t v = read_be32(in); write_le32(out, v); }
        if (o_u_flag & O_U_VAL7) { uint32_t v = read_be32(in); write_le32(out, v); }

        /* timer (INT[4]) */
        if (o_u_flag & O_U_TIMER) {
            for (i = 0; i < 4; i++) {
                uint32_t timer = read_be32(in);
                write_le32(out, timer);
            }
        }

        /* trap */
        if (o_u_flag & O_U_TRAP) {
            for (i = 0; i < 4; i++) {
                uint16_t val = read_be16(in);
                write_le16(out, val);
            }
        }

        /* type (BYTE) */
        if (o_u_flag & O_U_TYPE) {
            *(*out)++ = *(*in)++;
        }

        /* Flags */
        if (o_u_flag & O_U_WEAR) { uint32_t f = read_be32(in); write_le32(out, f); }
        if (o_u_flag & O_U_EXTRA) { uint32_t f = read_be32(in); write_le32(out, f); }
        if (o_u_flag & O_U_ANTI) { uint32_t f = read_be32(in); write_le32(out, f); }
        if (o_u_flag & O_U_ANTI2) { uint32_t f = read_be32(in); write_le32(out, f); }
        if (o_u_flag & O_U_EXTRA2) { uint32_t f = read_be32(in); write_le32(out, f); }

        /* weight, material, cost */
        if (o_u_flag & O_U_WEIGHT) { uint32_t w = read_be32(in); write_le32(out, w); }
        if (o_u_flag & O_U_MATERIAL) { *(*out)++ = *(*in)++; }
        if (o_u_flag & O_U_COST) { uint32_t c = read_be32(in); write_le32(out, c); }

        /* bitvectors */
        if (o_u_flag & O_U_BV1) { uint64_t bv = read_be64(in, header->long_size); write_le64(out, bv); }
        if (o_u_flag & O_U_BV2) { uint64_t bv = read_be64(in, header->long_size); write_le64(out, bv); }
        if (o_u_flag & O_U_BV3) { uint64_t bv = read_be64(in, header->long_size); write_le64(out, bv); }
        if (o_u_flag & O_U_BV4) { uint64_t bv = read_be64(in, header->long_size); write_le64(out, bv); }
        if (o_u_flag & O_U_BV5) { uint64_t bv = read_be64(in, header->long_size); write_le64(out, bv); }

        /* affects */
        if (o_u_flag & O_U_AFFS) {
            for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                *(*out)++ = *(*in)++;  /* location */
                *(*out)++ = *(*in)++;  /* modifier */
            }
        }
    }

    /* Optional: spellbook */
    if (o_f_flag & O_F_SPELLBOOK) {
        uint32_t spell_size = read_be32(in);
        if (spell_size > 10000 || *in + spell_size > input_end) {
            fprintf(stderr, "ERROR: Invalid spellbook size %u at depth %d\n", spell_size, depth);
            return 0;
        }
        write_le32(out, spell_size);
        for (i = 0; i < (int)spell_size; i++) {
            *(*out)++ = *(*in)++;
        }
    }

    /* Optional: container contents (recursive) */
    if (o_f_flag & O_F_CONTAINS) {
        if (verbose_mode) {
            fprintf(stderr, "DEBUG: depth=%d processing container contents\n", depth);
        }
        int container_items = 0;
        while (1) {
            if (*in >= input_end) {
                fprintf(stderr, "ERROR: Ran out of data while processing container at depth %d\n", depth);
                return 0;
            }

            uint8_t next_flag = **in;
            if (next_flag == O_F_EOL) {
                *(*out)++ = *(*in)++;  /* Copy EOL marker */
                if (verbose_mode) {
                    fprintf(stderr, "DEBUG: depth=%d found EOL in container after %d items\n", depth, container_items);
                }
                break;
            }

            if (++container_items > 1000) {
                fprintf(stderr, "ERROR: Too many items in container (possible infinite loop) at depth %d\n", depth);
                return 0;
            }

            if (!convert_item(in, out, header, depth + 1)) {
                return 0;
            }
        }
    }

    return 1;
}

/***************************************************************************
 * File I/O Functions
 ***************************************************************************/

static unsigned char* load_file(const char *filename, size_t *file_size) {
    FILE *fp;
    struct stat st;
    unsigned char *buf;

    if (stat(filename, &st) != 0) {
        fprintf(stderr, "ERROR: Cannot stat file '%s': %s\n", filename, strerror(errno));
        return NULL;
    }

    *file_size = st.st_size;

    if (*file_size > SAV_MAXSIZE) {
        fprintf(stderr, "ERROR: File too large (%zu bytes, max %d)\n",
                *file_size, SAV_MAXSIZE);
        return NULL;
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open file '%s': %s\n", filename, strerror(errno));
        return NULL;
    }

    buf = (unsigned char*)malloc(*file_size);
    if (!buf) {
        fprintf(stderr, "ERROR: Cannot allocate memory for file\n");
        fclose(fp);
        return NULL;
    }

    if (fread(buf, 1, *file_size, fp) != *file_size) {
        fprintf(stderr, "ERROR: Failed to read file\n");
        free(buf);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return buf;
}

static int save_file(const char *filename, unsigned char *data, size_t size) {
    FILE *fp;

    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot create file '%s': %s\n", filename, strerror(errno));
        return 0;
    }

    if (fwrite(data, 1, size, fp) != size) {
        fprintf(stderr, "ERROR: Failed to write file\n");
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

static int create_backup(const char *filename) {
    char backup_name[1024];
    unsigned char *data;
    size_t size;

    snprintf(backup_name, sizeof(backup_name), "%s.backup", filename);

    /* Check if backup already exists */
    if (access(backup_name, F_OK) == 0) {
        printf("Backup file '%s' already exists, skipping...\n", backup_name);
        return 1;
    }

    data = load_file(filename, &size);
    if (!data) {
        return 0;
    }

    if (!save_file(backup_name, data, size)) {
        free(data);
        return 0;
    }

    free(data);
    printf("Created backup: %s\n", backup_name);
    return 1;
}

/***************************************************************************
 * Main Program
 ***************************************************************************/

static void print_usage(const char *prog_name) {
    printf("DurisMUD Player File Converter\n\n");
    printf("Converts player files from big-endian (old format) to little-endian (new format)\n\n");
    printf("Usage: %s [OPTIONS] <input_file> [output_file]\n\n", prog_name);
    printf("Options:\n");
    printf("  -v, --verbose    Verbose diagnostic output\n");
    printf("  --dry-run        Validate input only, don't write output\n");
    printf("  --backup         Create .backup file before conversion\n");
    printf("  -h, --help       Show this help message\n\n");
    printf("If output_file is not specified, input file will be overwritten.\n");
    printf("Use --backup to create a safety copy before conversion.\n\n");
    printf("Examples:\n");
    printf("  %s Players/a/arih                    # Convert in-place\n", prog_name);
    printf("  %s --backup Players/a/arih           # Convert with backup\n", prog_name);
    printf("  %s -v Players/a/arih arih.new        # Convert to new file with diagnostics\n", prog_name);
    printf("  %s --dry-run Players/a/arih          # Validate only\n", prog_name);
}

int main(int argc, char **argv) {
    const char *input_file = NULL;
    const char *output_file = NULL;
    unsigned char *input_buf = NULL;
    unsigned char *output_buf = NULL;
    size_t input_size, output_size;
    int i;

    /* Initialize stats */
    memset(&stats, 0, sizeof(stats));

    /* Parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose_mode = 1;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            dry_run_mode = 1;
        } else if (strcmp(argv[i], "--backup") == 0) {
            backup_mode = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (!input_file) {
            input_file = argv[i];
        } else if (!output_file) {
            output_file = argv[i];
        } else {
            fprintf(stderr, "ERROR: Too many arguments\n");
            print_usage(argv[0]);
            return 1;
        }
    }

    if (!input_file) {
        fprintf(stderr, "ERROR: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }

    /* If no output file, overwrite input */
    if (!output_file) {
        output_file = input_file;
    }

    printf("=== DurisMUD Player File Converter ===\n");
    printf("Input:  %s\n", input_file);
    if (!dry_run_mode) {
        printf("Output: %s\n", output_file);
    } else {
        printf("Mode:   DRY RUN (validation only)\n");
    }

    /* Create backup if requested */
    if (backup_mode && !dry_run_mode) {
        if (!create_backup(input_file)) {
            return 1;
        }
    }

    /* Load input file */
    input_buf = load_file(input_file, &input_size);
    if (!input_buf) {
        return 1;
    }

    printf("Loaded %zu bytes\n", input_size);

    /* Convert */
    if (!convert_pfile(input_buf, input_size, &output_buf, &output_size)) {
        fprintf(stderr, "ERROR: Conversion failed\n");
        free(input_buf);
        return 1;
    }

    /* Print statistics */
    printf("\n=== Statistics ===\n");
    printf("Status section:  %d bytes read, %d bytes written\n",
           stats.status_bytes_read, stats.status_bytes_written);
    printf("Skills:          %d converted\n", stats.skills_converted);
    printf("Witness records: %d converted\n", stats.witness_records);
    printf("Affects:         %d converted\n", stats.affects_converted);
    printf("Items:           %d converted\n", stats.items_converted);

    /* Save output file */
    if (!dry_run_mode) {
        printf("\nWriting output file...\n");
        if (!save_file(output_file, output_buf, output_size)) {
            free(input_buf);
            free(output_buf);
            return 1;
        }
        printf("SUCCESS: Conversion complete!\n");
        printf("Output file: %s (%zu bytes)\n", output_file, output_size);
    } else {
        printf("\nDRY RUN: Validation successful, no files written\n");
    }

    free(input_buf);
    free(output_buf);

    return 0;
}
