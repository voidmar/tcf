#include <vector>
using std::vector;

#include <string>
using std::wstring;

#include <Windows.h>
#include <UserEnv.h>
#include <processthreadsapi.h>

// what is: tool containment field - start a .bat tool in a cursed environment with working process tree containment field

int wmain(int argc, const wchar_t* argv[])
{
    if (argc < 2)
        return 1;

    HANDLE job = CreateJobObject(NULL, NULL);
    if (!job)
        return GetLastError();

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit_info{};
    limit_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

    if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limit_info, sizeof(limit_info)))
        return GetLastError();

    if (!AssignProcessToJobObject(job, GetCurrentProcess()))
        return GetLastError();

    wstring command_line{ L"cmd.exe /D /C \"" };
    for (int i = 1; i < argc; ++i)
    {
        command_line.append(argv[i]);
        command_line.append(L" ");
    }
    command_line.back() = '\"';

    //command_line = L"";

    HANDLE access_token{ INVALID_HANDLE_VALUE };
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &access_token))
        return GetLastError();

    void* environment{ nullptr };
    if (!CreateEnvironmentBlock(&environment, access_token, false))
        return GetLastError();

    vector<wchar_t> command_line_mutable;
    command_line_mutable.insert(command_line_mutable.begin(), command_line.begin(), command_line.end());
    command_line_mutable.push_back(L'\0');

    //printf("%S", command_line_mutable.data());

    STARTUPINFO si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(NULL, command_line_mutable.data(), NULL, NULL, FALSE, CREATE_UNICODE_ENVIRONMENT, environment, NULL, &si, &pi))
        return GetLastError();

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD wait_exit_code{ 0 };
    if (!GetExitCodeProcess(pi.hProcess, &wait_exit_code))
        return GetLastError();

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    DestroyEnvironmentBlock(environment);

    for (;;)
    {
        JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting_info{};
        if (!QueryInformationJobObject(job, JobObjectBasicAccountingInformation, &accounting_info, sizeof(accounting_info), NULL))
            return GetLastError();

        if (accounting_info.ActiveProcesses == 1)
            break;
    }

    return wait_exit_code;
    // this code relies on automatic handle close on process terminate to clean up other processes entirely
}
