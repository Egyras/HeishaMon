#include "crash_helper.h"
#include "user_interface.h"
#include "Esp.h"
#include "string.h"

/*  EXCCAUSE register values:  */
/*
 *  General Exception Causes
 *  (values of EXCCAUSE special register set by general exceptions,
 *   which vector to the user, kernel, or double-exception vectors).
 */
#define EXCCAUSE_ILLEGAL 0          /* Illegal Instruction */
#define EXCCAUSE_SYSCALL 1          /* System Call (SYSCALL instruction) */
#define EXCCAUSE_INSTR_ERROR 2      /* Instruction Fetch Error */
#define EXCCAUSE_IFETCHERROR 2      /* (backward compatibility macro, deprecated, avoid) */
#define EXCCAUSE_LOAD_STORE_ERROR 3 /* Load Store Error */
#define EXCCAUSE_LOADSTOREERROR 3   /* (backward compatibility macro, deprecated, avoid) */
#define EXCCAUSE_LEVEL1_INTERRUPT 4 /* Level 1 Interrupt */
#define EXCCAUSE_LEVEL1INTERRUPT 4  /* (backward compatibility macro, deprecated, avoid) */
#define EXCCAUSE_ALLOCA 5           /* Stack Extension Assist (MOVSP instruction) for alloca */
#define EXCCAUSE_DIVIDE_BY_ZERO 6   /* Integer Divide by Zero */
#define EXCCAUSE_SPECULATION 7      /* Use of Failed Speculative Access (not implemented) */
#define EXCCAUSE_PRIVILEGED 8       /* Privileged Instruction */
#define EXCCAUSE_UNALIGNED 9        /* Unaligned Load or Store */
 /* Reserved				10..11 */
#define EXCCAUSE_INSTR_DATA_ERROR 12      /* PIF Data Error on Instruction Fetch (RB-200x and later) */
#define EXCCAUSE_LOAD_STORE_DATA_ERROR 13 /* PIF Data Error on Load or Store (RB-200x and later) */
#define EXCCAUSE_INSTR_ADDR_ERROR 14      /* PIF Address Error on Instruction Fetch (RB-200x and later) */
#define EXCCAUSE_LOAD_STORE_ADDR_ERROR 15 /* PIF Address Error on Load or Store (RB-200x and later) */
#define EXCCAUSE_ITLB_MISS 16             /* ITLB Miss (no ITLB entry matches, hw refill also missed) */
#define EXCCAUSE_ITLB_MULTIHIT 17         /* ITLB Multihit (multiple ITLB entries match) */
#define EXCCAUSE_INSTR_RING 18            /* Ring Privilege Violation on Instruction Fetch */
/* Reserved				19 */             /* Size Restriction on IFetch (not implemented) */
#define EXCCAUSE_INSTR_PROHIBITED 20      /* Cache Attribute does not allow Instruction Fetch */
/* Reserved				21..23 */
#define EXCCAUSE_DTLB_MISS 24        /* DTLB Miss (no DTLB entry matches, hw refill also missed) */
#define EXCCAUSE_DTLB_MULTIHIT 25    /* DTLB Multihit (multiple DTLB entries match) */
#define EXCCAUSE_LOAD_STORE_RING 26  /* Ring Privilege Violation on Load or Store */
/* Reserved				27 */        /* Size Restriction on Load/Store (not implemented) */
#define EXCCAUSE_LOAD_PROHIBITED 28  /* Cache Attribute does not allow Load */
#define EXCCAUSE_STORE_PROHIBITED 29 /* Cache Attribute does not allow Store */
/* Reserved				30..31 */
#define EXCCAUSE_CP_DISABLED(n) (32 + (n)) /* Access to Coprocessor 'n' when disabled */
#define EXCCAUSE_CP0_DISABLED 32           /* Access to Coprocessor 0 when disabled */
#define EXCCAUSE_CP1_DISABLED 33           /* Access to Coprocessor 1 when disabled */
#define EXCCAUSE_CP2_DISABLED 34           /* Access to Coprocessor 2 when disabled */
#define EXCCAUSE_CP3_DISABLED 35           /* Access to Coprocessor 3 when disabled */
#define EXCCAUSE_CP4_DISABLED 36           /* Access to Coprocessor 4 when disabled */
#define EXCCAUSE_CP5_DISABLED 37           /* Access to Coprocessor 5 when disabled */
#define EXCCAUSE_CP6_DISABLED 38           /* Access to Coprocessor 6 when disabled */
#define EXCCAUSE_CP7_DISABLED 39           /* Access to Coprocessor 7 when disabled */

static const uint8_t RTC_MEMORY_OFFSET = 4; // The DoubleResetDetect library is using the first 4 bytes of the RTC memory.

typedef struct
{
    uint32_t magic;
    uint32_t crash_times;
    uint8_t restart_reason;
    uint8_t exception_cause;
    uint32_t epc1;
    uint32_t epc2;
    uint32_t epc3;
    uint32_t excvaddr;
    uint32_t depc;
    uint32_t stack_start;
    uint32_t stack_end;
    uint8_t stack_trace[200];
} crash_helper_context_t;

const uint32_t CRASH_HELPER_MAGIC = 0xdeadbeef;

static crash_helper_context_t loaded_context_cache;
static bool has_loaded_context_cache = false;

static void load_crash_context()
{
    if (has_loaded_context_cache) {
        return;
    }

    ESP.rtcUserMemoryRead(RTC_MEMORY_OFFSET, (uint32_t *)&loaded_context_cache, sizeof(crash_helper_context_t));
    if (loaded_context_cache.magic != CRASH_HELPER_MAGIC) {
        loaded_context_cache.magic = CRASH_HELPER_MAGIC;
        loaded_context_cache.crash_times = 0;
        loaded_context_cache.restart_reason = 0;
        loaded_context_cache.exception_cause = 0;
        loaded_context_cache.epc1 = 0;
        loaded_context_cache.epc2 = 0;
        loaded_context_cache.epc3 = 0;
        loaded_context_cache.excvaddr = 0;
        loaded_context_cache.depc = 0;
        loaded_context_cache.stack_start = 0;
        loaded_context_cache.stack_end = 0;
        memset(loaded_context_cache.stack_trace, 0, sizeof(loaded_context_cache.stack_trace));
    }

    has_loaded_context_cache = true;
}

static void save_crash_context()
{
    ESP.rtcUserMemoryWrite(RTC_MEMORY_OFFSET, (uint32_t *)&loaded_context_cache, sizeof(crash_helper_context_t));
}

extern "C" void custom_crash_callback(struct rst_info *rst_info, uint32_t stack, uint32_t stack_end)
{
    load_crash_context();

    loaded_context_cache.crash_times++;
    loaded_context_cache.restart_reason = rst_info->reason;
    loaded_context_cache.exception_cause = rst_info->exccause;
    loaded_context_cache.epc1 = rst_info->epc1;
    loaded_context_cache.epc2 = rst_info->epc2;
    loaded_context_cache.epc3 = rst_info->epc3;
    loaded_context_cache.excvaddr = rst_info->excvaddr;
    loaded_context_cache.depc = rst_info->depc;
    loaded_context_cache.stack_start = stack;
    loaded_context_cache.stack_end = stack_end;
    const uint16_t stack_size = stack_end - stack;

    memcpy(loaded_context_cache.stack_trace, (void *)stack, stack_size);

    save_crash_context();
}

bool crash_helper_has_crashed()
{
    load_crash_context();
    return loaded_context_cache.crash_times != 0;
}

void crash_helper_clear_crash()
{
    load_crash_context();
    loaded_context_cache.crash_times = 0;
    save_crash_context();
}

void crash_helper_print_crash(char *buffer, uint16_t buffer_size)
{
    load_crash_context();

    if (loaded_context_cache.crash_times == 0) {
        snprintf(buffer, buffer_size, "No crash");
        return;
    }

    const char *restart_reason = "Unknown";
    switch (loaded_context_cache.restart_reason) {
    case REASON_DEFAULT_RST:
        restart_reason = "Default";
        break;
    case REASON_WDT_RST:
        restart_reason = "Watchdog";
        break;
    case REASON_EXCEPTION_RST:
        restart_reason = "Exception";
        break;
    case REASON_SOFT_WDT_RST:
        restart_reason = "Soft watchdog";
        break;
    case REASON_SOFT_RESTART:
        restart_reason = "Soft restart";
        break;
    case REASON_DEEP_SLEEP_AWAKE:
        restart_reason = "Deep sleep";
        break;
    case REASON_EXT_SYS_RST:
        restart_reason = "External";
        break;
    }

    const char *exception_cause = "Unknown";
    switch (loaded_context_cache.exception_cause) {
    case EXCCAUSE_ILLEGAL: // IllegalInstructionCause
        exception_cause = "Illegal";
        break;
    case EXCCAUSE_SYSCALL: // SyscallCause
        exception_cause = "Syscall";
        break;
    case EXCCAUSE_INSTR_ERROR: // InstructionFetchErrorCause
        exception_cause = "Instruction fetch";
        break;
    case EXCCAUSE_LOAD_STORE_ERROR: // LoadStoreErrorCause
        exception_cause = "Load/store";
        break;
    case EXCCAUSE_ALLOCA: // Level1InterruptCause
        exception_cause = "Level1";
        break;
    case EXCCAUSE_DIVIDE_BY_ZERO: // AllocaCause
        exception_cause = "Alloca";
        break;
    case EXCCAUSE_SPECULATION: // IntegerDivideByZeroCause
        exception_cause = "Integer divide by zero";
        break;
    case EXCCAUSE_PRIVILEGED: // PrivilegedCause
        exception_cause = "Privileged";
        break;
    case EXCCAUSE_LOAD_PROHIBITED: // LoadStoreAlignmentCause
        exception_cause = "Load prohibited";
        break;
    case EXCCAUSE_STORE_PROHIBITED: // StoreProhibitedCause
        exception_cause = "Store prohibited";
        break;
    case EXCCAUSE_UNALIGNED:
        exception_cause = "Unaligned";
        break;
    case EXCCAUSE_INSTR_DATA_ERROR:
        exception_cause = "Instruction data";
        break;
    case EXCCAUSE_LOAD_STORE_DATA_ERROR:
        exception_cause = "Load/store data";
        break;
    case EXCCAUSE_INSTR_ADDR_ERROR:
        exception_cause = "Instruction address";
        break;
    case EXCCAUSE_LOAD_STORE_ADDR_ERROR:
        exception_cause = "Load/store address";
        break;
    case EXCCAUSE_ITLB_MISS:
        exception_cause = "ITLB miss";
        break;
    case EXCCAUSE_ITLB_MULTIHIT:
        exception_cause = "ITLB multihit";
        break;
    case EXCCAUSE_INSTR_RING:
        exception_cause = "Instruction ring";
        break;
    case EXCCAUSE_INSTR_PROHIBITED:
        exception_cause = "Instruction prohibited";
        break;
    case EXCCAUSE_DTLB_MISS:
        exception_cause = "DTLB miss";
        break;
    case EXCCAUSE_DTLB_MULTIHIT:
        exception_cause = "DTLB multihit";
        break;
    case EXCCAUSE_LOAD_STORE_RING:
        exception_cause = "Load/store ring";
        break;
    case EXCCAUSE_CP0_DISABLED:
        exception_cause = "CP0 disabled";
        break;
    case EXCCAUSE_CP1_DISABLED:
        exception_cause = "CP1 disabled";
        break;
    case EXCCAUSE_CP2_DISABLED:
        exception_cause = "CP2 disabled";
        break;
    case EXCCAUSE_CP3_DISABLED:
        exception_cause = "CP3 disabled";
        break;
    case EXCCAUSE_CP4_DISABLED:
        exception_cause = "CP4 disabled";
        break;
    case EXCCAUSE_CP5_DISABLED:
        exception_cause = "CP5 disabled";
        break;
    case EXCCAUSE_CP6_DISABLED:
        exception_cause = "CP6 disabled";
        break;
    case EXCCAUSE_CP7_DISABLED:
        exception_cause = "CP7 disabled";
        break;
    }

    snprintf(buffer, buffer_size, "Restart reason: %s\nException cause: %s\nEPC1: 0x%08x\nEPC2: 0x%08x\nEPC3: 0x%08x\nEXCVADDR: 0x%08x\nDEPC: 0x%08x\nStack start: 0x%08x\nStack end: 0x%08x\n",
        restart_reason, exception_cause, loaded_context_cache.epc1, loaded_context_cache.epc2, loaded_context_cache.epc3, loaded_context_cache.excvaddr, loaded_context_cache.depc, loaded_context_cache.stack_start, loaded_context_cache.stack_end);

    uint16_t stack_trace_word_length = loaded_context_cache.stack_end - loaded_context_cache.stack_start;
    stack_trace_word_length = stack_trace_word_length > sizeof(loaded_context_cache.stack_trace) ? sizeof(loaded_context_cache.stack_trace) : stack_trace_word_length;

    stack_trace_word_length = stack_trace_word_length / 4; // Stack trace size in 32bit words.

    // Contatinate the stack addresses to the buffer
    uint32_t *stack_value_ptr = (uint32_t *)loaded_context_cache.stack_trace;
    for (int16_t i = 0; i < stack_trace_word_length; i += 4) {
        snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "%08x: ", loaded_context_cache.stack_start + i);

        uint16_t column_cnt = stack_trace_word_length - i;
        column_cnt = column_cnt > 4 ? 4 : column_cnt;

        for (byte j = 0; j < column_cnt; j++) {
            snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "%08x ", *stack_value_ptr);
            stack_value_ptr++;
        }
        snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "\n");
    }
}