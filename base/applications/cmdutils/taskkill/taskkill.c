/*
 * Task termination utility
 *
 * Copyright 2008 Andrew Riedi
 * Copyright 2010 Andrew Nguyen
 * Copyright 2020 He Yang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdlib.h>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <wine/debug.h>
#include <wine/unicode.h>

#include "taskkill.h"

WINE_DEFAULT_DEBUG_CHANNEL(taskkill);

static BOOL force_termination = FALSE;
static BOOL kill_child_processes = FALSE;

static WCHAR **task_list;
static unsigned int task_count;

#ifdef __REACTOS__

static WCHAR opForceTerminate[] = L"f";
static WCHAR opImage[] = L"im";
static WCHAR opPID[] = L"pid";
static WCHAR opHelp[] = L"?";
static WCHAR opTerminateChildren[] = L"t";

static PWCHAR opList[] = {opForceTerminate, opImage, opPID, opHelp, opTerminateChildren};

#define OP_PARAM_INVALID -1

#define OP_PARAM_FORCE_TERMINATE 0
#define OP_PARAM_IMAGE 1
#define OP_PARAM_PID 2
#define OP_PARAM_HELP 3
#define OP_PARAM_TERMINATE_CHILD 4

#endif // __REACTOS__

struct pid_close_info
{
    DWORD pid;
    BOOL found;
};

static int taskkill_vprintfW(const WCHAR *msg, __ms_va_list va_args)
{
    int wlen;
    DWORD count, ret;
    WCHAR msg_buffer[8192];

    wlen = FormatMessageW(FORMAT_MESSAGE_FROM_STRING, msg, 0, 0, msg_buffer,
                          ARRAY_SIZE(msg_buffer), &va_args);

    ret = WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), msg_buffer, wlen, &count, NULL);
    if (!ret)
    {
        DWORD len;
        char *msgA;

        /* On Windows WriteConsoleW() fails if the output is redirected. So fall
         * back to WriteFile(), assuming the console encoding is still the right
         * one in that case.
         */
        len = WideCharToMultiByte(GetConsoleOutputCP(), 0, msg_buffer, wlen,
            NULL, 0, NULL, NULL);
        msgA = HeapAlloc(GetProcessHeap(), 0, len);
        if (!msgA)
            return 0;

        WideCharToMultiByte(GetConsoleOutputCP(), 0, msg_buffer, wlen, msgA, len,
            NULL, NULL);
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), msgA, len, &count, FALSE);
        HeapFree(GetProcessHeap(), 0, msgA);
    }

    return count;
}

static int WINAPIV taskkill_printfW(const WCHAR *msg, ...)
{
    __ms_va_list va_args;
    int len;

    __ms_va_start(va_args, msg);
    len = taskkill_vprintfW(msg, va_args);
    __ms_va_end(va_args);

    return len;
}

static int WINAPIV taskkill_message_printfW(int msg, ...)
{
    __ms_va_list va_args;
    WCHAR msg_buffer[8192];
    int len;

    LoadStringW(GetModuleHandleW(NULL), msg, msg_buffer, ARRAY_SIZE(msg_buffer));

    __ms_va_start(va_args, msg);
    len = taskkill_vprintfW(msg_buffer, va_args);
    __ms_va_end(va_args);

    return len;
}

static int taskkill_message(int msg)
{
    static const WCHAR formatW[] = {'%','1',0};
    WCHAR msg_buffer[8192];

    LoadStringW(GetModuleHandleW(NULL), msg, msg_buffer, ARRAY_SIZE(msg_buffer));

    return taskkill_printfW(formatW, msg_buffer);
}

/* Post WM_CLOSE to all top-level windows belonging to the process with specified PID. */
static BOOL CALLBACK pid_enum_proc(HWND hwnd, LPARAM lParam)
{
    struct pid_close_info *info = (struct pid_close_info *)lParam;
    DWORD hwnd_pid;

    GetWindowThreadProcessId(hwnd, &hwnd_pid);

    if (hwnd_pid == info->pid)
    {
        PostMessageW(hwnd, WM_CLOSE, 0, 0);
        info->found = TRUE;
    }

    return TRUE;
}

static DWORD *enumerate_processes(DWORD *list_count)
{
    DWORD *pid_list, alloc_bytes = 1024 * sizeof(*pid_list), needed_bytes;

    pid_list = HeapAlloc(GetProcessHeap(), 0, alloc_bytes);
    if (!pid_list)
        return NULL;

    for (;;)
    {
        DWORD *realloc_list;

        if (!EnumProcesses(pid_list, alloc_bytes, &needed_bytes))
        {
            HeapFree(GetProcessHeap(), 0, pid_list);
            return NULL;
        }

        /* EnumProcesses can't signal an insufficient buffer condition, so the
         * only way to possibly determine whether a larger buffer is required
         * is to see whether the written number of bytes is the same as the
         * buffer size. If so, the buffer will be reallocated to twice the
         * size. */
        if (alloc_bytes != needed_bytes)
            break;

        alloc_bytes *= 2;
        realloc_list = HeapReAlloc(GetProcessHeap(), 0, pid_list, alloc_bytes);
        if (!realloc_list)
        {
            HeapFree(GetProcessHeap(), 0, pid_list);
            return NULL;
        }
        pid_list = realloc_list;
    }

    *list_count = needed_bytes / sizeof(*pid_list);
    return pid_list;
}

static BOOL get_process_name_from_pid(DWORD pid, WCHAR *buf, DWORD chars)
{
    HANDLE process;
    HMODULE module;
    DWORD required_size;

    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process)
        return FALSE;

    if (!EnumProcessModules(process, &module, sizeof(module), &required_size))
    {
        CloseHandle(process);
        return FALSE;
    }

    if (!GetModuleBaseNameW(process, module, buf, chars))
    {
        CloseHandle(process);
        return FALSE;
    }

    CloseHandle(process);
    return TRUE;
}

/* The implemented task enumeration and termination behavior does not
 * exactly match native behavior. On Windows:
 *
 * In the case of terminating by process name, specifying a particular
 * process name more times than the number of running instances causes
 * all instances to be terminated, but termination failure messages to
 * be printed as many times as the difference between the specification
 * quantity and the number of running instances.
 *
 * Successful terminations are all listed first in order, with failing
 * terminations being listed at the end.
 *
 * A PID of zero causes taskkill to warn about the inability to terminate
 * system processes. */


static BOOL get_pid_creation_time(DWORD pid, FILETIME *time)
{
    HANDLE process = INVALID_HANDLE_VALUE;
    FILETIME t1 = { 0 }, t2 = { 0 }, t3 = { 0 };

    process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!process)
    {
        return FALSE;
    }

    if (!GetProcessTimes(process, time, &t1, &t2, &t3))
    {
        CloseHandle(process);
        return FALSE;
    }

    CloseHandle(process);
    return TRUE;
}

static void send_close_messages_tree(DWORD ppid)
{
    FILETIME parent_creation_time = { 0 };
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (!get_pid_creation_time(ppid, &parent_creation_time) || !h)
    {
        CloseHandle(h);
        return;
    }

    if (Process32FirstW(h, &pe))
    {
        do
        {
            FILETIME child_creation_time = { 0 };
            struct pid_close_info info = { pe.th32ProcessID };

            if (!get_pid_creation_time(pe.th32ProcessID, &child_creation_time))
            {
                continue;
            }

            // Compare creation time to avoid reuse PID, thanks to @ThFabba
            if (pe.th32ParentProcessID == ppid &&
                CompareFileTime(&parent_creation_time, &child_creation_time) < 0)
            {
                // Use recursion to browse all child processes
                send_close_messages_tree(pe.th32ProcessID);
                EnumWindows(pid_enum_proc, (LPARAM)&info);
                if (info.found)
                {
                    taskkill_message_printfW(STRING_CLOSE_CHILD, pe.th32ProcessID, ppid);
                }
            }
        } while (Process32NextW(h, &pe));
    }

    CloseHandle(h);
}

static int send_close_messages(void)
{
    DWORD *pid_list, pid_list_size, *pkill_list;
    DWORD self_pid = GetCurrentProcessId();
    unsigned int i;
    int status_code = 0;

    pid_list = enumerate_processes(&pid_list_size);
    if (!pid_list)
    {
        taskkill_message(STRING_ENUM_FAILED);
        return 1;
    }

    pkill_list = HeapAlloc(GetProcessHeap(), 0, pid_list_size * sizeof(DWORD));
    if (!pkill_list)
        return 1;

    for (i = 0; i < task_count; i++)
    {
        WCHAR *p = task_list[i];
        BOOL is_numeric = TRUE;
        DWORD pkill_size = 0, index = 0;

        memset(pkill_list, 0, pid_list_size * sizeof(DWORD));

        /* Determine whether the string is not numeric. */
        while (*p)
        {
            if (!isdigitW(*p++))
            {
                is_numeric = FALSE;
                break;
            }
        }

        // Find processes to kill
        if (is_numeric)
        {
            WCHAR ps_name[MAX_PATH] = { 0 };
            if (get_process_name_from_pid(atoiW(task_list[i]), ps_name, MAX_PATH))
            {
                pkill_list[pkill_size] = atoiW(task_list[i]);
                pkill_size++;
            }
        }
        else
        {
            for (index = 0; index < pid_list_size; index++)
            {
                WCHAR process_name[MAX_PATH];
                if (get_process_name_from_pid(pid_list[index], process_name, MAX_PATH) &&
                    !strcmpiW(process_name, task_list[i]))
                {
                    pkill_list[pkill_size] = pid_list[index];
                    pkill_size++;
                }
            }
        }

        // Can't find any process same as name or PID
        if (pkill_size == 0)
        {
            taskkill_message_printfW(STRING_SEARCH_FAILED, task_list[i]);
            status_code = 128;
            continue;
        }

        // Try to send close messages to process in `pkill_list`
        for (index = 0; index < pkill_size; index++)
        {
            DWORD pid = pkill_list[index];
            WCHAR process_name[MAX_PATH] = { 0 };
            struct pid_close_info info = { pid };

            if (pid == self_pid)
            {
                taskkill_message(STRING_SELF_TERMINATION);
                status_code = 1;
                continue;
            }

            // Send close messages to child first
            if (kill_child_processes)
            {
                send_close_messages_tree(pid);
            }

            get_process_name_from_pid(pid, process_name, MAX_PATH);
            EnumWindows(pid_enum_proc, (LPARAM)&info);
            if (info.found)
            {
                if (is_numeric)
                {
                    taskkill_message_printfW(STRING_CLOSE_PID_SEARCH, pid);
                }
                else
                {
                    taskkill_message_printfW(STRING_CLOSE_PROC_SRCH, process_name, pid);
                }
            }
        }
    }

    HeapFree(GetProcessHeap(), 0, pkill_list);
    HeapFree(GetProcessHeap(), 0, pid_list);
    return status_code;
}

static void terminate_process_tree(DWORD ppid)
{
    FILETIME parent_creation_time = { 0 };
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = { 0 };
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (!get_pid_creation_time(ppid, &parent_creation_time) || !h)
    {
        CloseHandle(h);
        return;
    }

    if (Process32FirstW(h, &pe))
    {
        do
        {
            FILETIME child_creation_time = { 0 };

            if (!get_pid_creation_time(pe.th32ProcessID, &child_creation_time))
            {
                continue;
            }

            // Compare creation time to avoid reuse PID, thanks to @ThFabba
            if (pe.th32ParentProcessID == ppid &&
                CompareFileTime(&parent_creation_time, &child_creation_time) < 0)
            {
                HANDLE process = INVALID_HANDLE_VALUE;

                // Use recursion to browse all child processes
                terminate_process_tree(pe.th32ProcessID);
                process = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (!process)
                {
                    continue;
                }

                if (!TerminateProcess(process, 0))
                {
                    taskkill_message_printfW(STRING_TERM_CHILD_FAILED, pe.th32ProcessID, ppid);
                    CloseHandle(process);
                    continue;
                }

                taskkill_message_printfW(STRING_TERM_CHILD, pe.th32ProcessID, ppid);
                CloseHandle(process);
            }
        } while (Process32NextW(h, &pe));
    }

    CloseHandle(h);
}

static int terminate_processes(void)
{
    DWORD *pid_list, pid_list_size, *pkill_list;
    DWORD self_pid = GetCurrentProcessId();
    unsigned int i;
    int status_code = 0;

    pid_list = enumerate_processes(&pid_list_size);
    if (!pid_list)
    {
        taskkill_message(STRING_ENUM_FAILED);
        return 1;
    }

    pkill_list = HeapAlloc(GetProcessHeap(), 0, pid_list_size * sizeof(DWORD));
    if (!pkill_list)
        return 1;

    for (i = 0; i < task_count; i++)
    {
        WCHAR *p = task_list[i];
        BOOL is_numeric = TRUE;
        DWORD pkill_size = 0, index = 0;

        memset(pkill_list, 0, pid_list_size * sizeof(DWORD));

        /* Determine whether the string is not numeric. */
        while (*p)
        {
            if (!isdigitW(*p++))
            {
                is_numeric = FALSE;
                break;
            }
        }

        // Find processes to kill
        if (is_numeric)
        {
            WCHAR ps_name[MAX_PATH] = { 0 };
            if (get_process_name_from_pid(atoiW(task_list[i]), ps_name, MAX_PATH))
            {
                pkill_list[pkill_size] = atoiW(task_list[i]);
                pkill_size++;
            }
        }
        else
        {
            for (index = 0; index < pid_list_size; index++)
            {
                WCHAR process_name[MAX_PATH];
                if (get_process_name_from_pid(pid_list[index], process_name, MAX_PATH) &&
                    !strcmpiW(process_name, task_list[i]))
                {
                    pkill_list[pkill_size] = pid_list[index];
                    pkill_size++;
                }
            }
        }

        // Can't find any process same as name or PID
        if (!pkill_size)
        {
            taskkill_message_printfW(STRING_SEARCH_FAILED, task_list[i]);
            status_code = 128;
            continue;
        }

        // Try to terminate to process in `pkill_list`
        for (index = 0; index < pkill_size; index++)
        {
            DWORD pid = pkill_list[index];
            WCHAR process_name[MAX_PATH] = { 0 };
            HANDLE process = INVALID_HANDLE_VALUE;

            if (pid == self_pid)
            {
                taskkill_message(STRING_SELF_TERMINATION);
                status_code = 1;
                continue;
            }

            // Terminate child first
            if (kill_child_processes)
            {
                terminate_process_tree(pid);
            }

            process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
            if (get_process_name_from_pid(pid, process_name, MAX_PATH) && !process)
            {
                taskkill_message_printfW(STRING_SEARCH_FAILED, task_list[i]);
                status_code = 128;
                continue;
            }

            if (!TerminateProcess(process, 0))
            {
                taskkill_message_printfW(STRING_TERMINATE_FAILED, process_name);
                status_code = 1;
                CloseHandle(process);
                continue;
            }

            if (is_numeric)
            {
                taskkill_message_printfW(STRING_TERM_PID_SEARCH, pid);
            }
            else
            {
                taskkill_message_printfW(STRING_TERM_PROC_SEARCH, process_name, pid);
            }
            CloseHandle(process);
        }
    }

    HeapFree(GetProcessHeap(), 0, pkill_list);
    HeapFree(GetProcessHeap(), 0, pid_list);
    return status_code;
}

static BOOL add_to_task_list(WCHAR *name)
{
    static unsigned int list_size = 16;

    if (!task_list)
    {
        task_list = HeapAlloc(GetProcessHeap(), 0,
                                   list_size * sizeof(*task_list));
        if (!task_list)
            return FALSE;
    }
    else if (task_count == list_size)
    {
        void *realloc_list;

        list_size *= 2;
        realloc_list = HeapReAlloc(GetProcessHeap(), 0, task_list,
                                   list_size * sizeof(*task_list));
        if (!realloc_list)
            return FALSE;

        task_list = realloc_list;
    }

    task_list[task_count++] = name;
    return TRUE;
}

#ifdef __REACTOS__

static int get_argument_type(WCHAR* argument)
{
    int i;

    if (argument[0] != L'/' && argument[0] != L'-')
    {
        return OP_PARAM_INVALID;
    }
    argument++;

    for (i = 0; i < _countof(opList); i++)
    {
        if (!strcmpiW(opList[i], argument))
        {
            return i;
        }
    }
    return OP_PARAM_INVALID;
}

static BOOL process_arguments(int argc, WCHAR* argv[])
{
    BOOL has_im = FALSE, has_pid = FALSE, has_help = FALSE;

    if (argc > 1)
    {
        int i;
        for (i = 1; i < argc; i++)
        {
            int argument = get_argument_type(argv[i]);

            switch (argument)
            {
            case OP_PARAM_FORCE_TERMINATE:
            {
                if (force_termination == TRUE)
                {
                    // -f already specified
                    taskkill_message_printfW(STRING_PARAM_TOO_MUCH, argv[i], 1);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }
                force_termination = TRUE;
                break;
            }
            case OP_PARAM_IMAGE:
            case OP_PARAM_PID:
            {
                if (!argv[i + 1])
                {
                    taskkill_message_printfW(STRING_MISSING_PARAM, argv[i]);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }

                if (argument == OP_PARAM_IMAGE)
                    has_im = TRUE;
                if (argument == OP_PARAM_PID)
                    has_pid = TRUE;

                if (has_im && has_pid)
                {
                    taskkill_message(STRING_MUTUAL_EXCLUSIVE);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }

                if (get_argument_type(argv[i + 1]) != OP_PARAM_INVALID)
                {
                    taskkill_message_printfW(STRING_MISSING_PARAM, argv[i]);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }

                if (!add_to_task_list(argv[++i])) // add next parameters to task_list
                    return FALSE;

                break;
            }
            case OP_PARAM_HELP:
            {
                if (has_help == TRUE)
                {
                    // -? already specified
                    taskkill_message_printfW(STRING_PARAM_TOO_MUCH, argv[i], 1);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }
                has_help = TRUE;
                break;
            }
            case OP_PARAM_TERMINATE_CHILD:
            {
                if (kill_child_processes == TRUE)
                {
                    // -t already specified
                    taskkill_message_printfW(STRING_PARAM_TOO_MUCH, argv[i], 1);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }
                kill_child_processes = TRUE;
                break;
            }
            case OP_PARAM_INVALID:
            default:
            {
                taskkill_message(STRING_INVALID_OPTION);
                taskkill_message(STRING_USAGE);
                return FALSE;
            }
            }
        }
    }

    if (has_help)
    {
        if (argc > 2) // any parameters other than -? is specified
        {
            taskkill_message(STRING_INVALID_SYNTAX);
            taskkill_message(STRING_USAGE);
            return FALSE;
        }
        else
        {
            taskkill_message(STRING_USAGE);
            exit(0);
        }
    }
    else if ((!has_im) && (!has_pid)) // has_help == FALSE
    {
        // both has_im and has_pid are missing (maybe -fi option is missing too, if implemented later)
        taskkill_message(STRING_MISSING_OPTION);
        taskkill_message(STRING_USAGE);
        return FALSE;
    }

    return TRUE;
}

#else

/* FIXME Argument processing does not match behavior observed on Windows.
 * Stringent argument counting and processing is performed, and unrecognized
 * options are detected as parameters when placed after options that accept one. */
static BOOL process_arguments(int argc, WCHAR *argv[])
{
    static const WCHAR opForceTerminate[] = {'f',0};
    static const WCHAR opImage[] = {'i','m',0};
    static const WCHAR opPID[] = {'p','i','d',0};
    static const WCHAR opHelp[] = {'?',0};
    static const WCHAR opTerminateChildren[] = {'t',0};

    if (argc > 1)
    {
        int i;
        WCHAR *argdata;
        BOOL has_im = FALSE, has_pid = FALSE;

        /* Only the lone help option is recognized. */
        if (argc == 2)
        {
            argdata = argv[1];
            if ((*argdata == '/' || *argdata == '-') && !strcmpW(opHelp, argdata + 1))
            {
                taskkill_message(STRING_USAGE);
                exit(0);
            }
        }

        for (i = 1; i < argc; i++)
        {
            BOOL got_im = FALSE, got_pid = FALSE;

            argdata = argv[i];
            if (*argdata != '/' && *argdata != '-')
                goto invalid;
            argdata++;

            if (!strcmpiW(opTerminateChildren, argdata))
                kill_child_processes = TRUE;
            else if (!strcmpiW(opForceTerminate, argdata))
                force_termination = TRUE;
            /* Options /IM and /PID appear to behave identically, except for
             * the fact that they cannot be specified at the same time. */
            else if ((got_im = !strcmpiW(opImage, argdata)) ||
                     (got_pid = !strcmpiW(opPID, argdata)))
            {
                if (!argv[i + 1])
                {
                    taskkill_message_printfW(STRING_MISSING_PARAM, argv[i]);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }

                if (got_im) has_im = TRUE;
                if (got_pid) has_pid = TRUE;

                if (has_im && has_pid)
                {
                    taskkill_message(STRING_MUTUAL_EXCLUSIVE);
                    taskkill_message(STRING_USAGE);
                    return FALSE;
                }

                if (!add_to_task_list(argv[i + 1]))
                    return FALSE;
                i++;
            }
            else
            {
                invalid:
                taskkill_message(STRING_INVALID_OPTION);
                taskkill_message(STRING_USAGE);
                return FALSE;
            }
        }
    }
    else
    {
        taskkill_message(STRING_MISSING_OPTION);
        taskkill_message(STRING_USAGE);
        return FALSE;
    }

    return TRUE;
}

#endif // __REACTOS__

int wmain(int argc, WCHAR *argv[])
{
    int status_code = 0;

    if (!process_arguments(argc, argv))
    {
        HeapFree(GetProcessHeap(), 0, task_list);
        return 1;
    }

    if (force_termination)
        status_code = terminate_processes();
    else
        status_code = send_close_messages();

    HeapFree(GetProcessHeap(), 0, task_list);
    return status_code;
}
