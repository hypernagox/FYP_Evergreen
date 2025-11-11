#include "NagiocpXPch.h"
#include "CrashDump.h"
#include <shlobj.h>
#include <strsafe.h>
#include <csignal>
#include <process.h>
#pragma comment(lib, "Shell32.lib")

namespace NagiocpX
{
    constexpr const static MINIDUMP_TYPE g_kMaxType =
        (MINIDUMP_TYPE)(
            // 메모리/페이지
            MiniDumpWithFullMemory |
            MiniDumpWithFullMemoryInfo |
            MiniDumpWithPrivateReadWriteMemory |
            MiniDumpWithPrivateWriteCopyMemory |
            MiniDumpWithIndirectlyReferencedMemory |
            MiniDumpScanMemory |
            MiniDumpIgnoreInaccessibleMemory |

            // 코드/데이터/모듈
            MiniDumpWithDataSegs |
            MiniDumpWithCodeSegs |
            MiniDumpWithModuleHeaders |
            MiniDumpWithUnloadedModules |

            // 스레드/핸들/프로세스
            MiniDumpWithThreadInfo |
            MiniDumpWithHandleData |
            MiniDumpWithProcessThreadData |

            // 추가 상태
            MiniDumpWithFullAuxiliaryState |
            MiniDumpWithTokenInformation |
            MiniDumpWithAvxXStateContext |
            MiniDumpWithIptTrace
            );

    constexpr const static MINIDUMP_TYPE g_kFatMini =
        (MINIDUMP_TYPE)(
            MiniDumpWithFullMemoryInfo |
            MiniDumpWithHandleData |
            MiniDumpWithThreadInfo |
            MiniDumpWithUnloadedModules |
            MiniDumpWithDataSegs |
            MiniDumpWithCodeSegs |
            MiniDumpWithProcessThreadData |
            MiniDumpWithPrivateReadWriteMemory |
            MiniDumpWithPrivateWriteCopyMemory |
            MiniDumpWithIndirectlyReferencedMemory |
            MiniDumpScanMemory |
            MiniDumpWithModuleHeaders |
            MiniDumpIgnoreInaccessibleMemory |
            MiniDumpWithFullAuxiliaryState |
            MiniDumpWithTokenInformation |
            MiniDumpWithAvxXStateContext |
            MiniDumpWithIptTrace
            );

    static void BuildPrimaryDumpPath(wchar_t* outPath, size_t cchOut, DWORD code = 0, uintptr_t addr = 0, DWORD tid = 0)
    {
        // 일단 C드라이브
        wchar_t dir[MAX_PATH] = L"C:\\Dumps";
        ::SHCreateDirectoryExW(nullptr, dir, nullptr);

        SYSTEMTIME st; ::GetLocalTime(&st);
        ::StringCchPrintfW(outPath, cchOut,
            L"%s\\crash_%04u%02u%02u_%02u%02u%02u_pid%u_code%08X_addr%p_tid0x%08X.dmp",
            dir, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
            ::GetCurrentProcessId(), code, (void*)addr, tid);
    }

    static void BuildFallbackDumpPath(wchar_t* outPath, size_t cchOut, DWORD code = 0, uintptr_t addr = 0, DWORD tid = 0)
    {
        // 실패하면 내 AppData -> Local -> CrashDumps에
        wchar_t lad[MAX_PATH] = {};
        if (SUCCEEDED(::SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, lad)))
        {
            wchar_t dir[MAX_PATH] = {};
            ::StringCchPrintfW(dir, MAX_PATH, L"%s\\CrashDumps", lad);
            ::SHCreateDirectoryExW(nullptr, dir, nullptr);

            SYSTEMTIME st; ::GetLocalTime(&st);
            ::StringCchPrintfW(outPath, cchOut,
                L"%s\\crash_%04u%02u%02u_%02u%02u%02u_pid%u_code%08X_addr%p_tid0x%08X.dmp",
                dir, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                ::GetCurrentProcessId(), code, (void*)addr, tid);
        }
        else
        {
            wchar_t dir[MAX_PATH] = L"C:\\Temp";
            ::SHCreateDirectoryExW(nullptr, dir, nullptr);
            SYSTEMTIME st; ::GetLocalTime(&st);
            ::StringCchPrintfW(outPath, cchOut,
                L"%s\\crash_%04u%02u%02u_%02u%02u%02u_pid%u_code%08X_addr%p_tid0x%08X.dmp",
                dir, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                ::GetCurrentProcessId(), code, (void*)addr, tid);
        }
    }

    static HANDLE OpenDumpFileWithFallback(wchar_t* usedPath, size_t cchPath, DWORD code = 0, uintptr_t addr = 0, DWORD tid = 0)
    {
        BuildPrimaryDumpPath(usedPath, cchPath, code, addr, tid);
        HANDLE h = ::CreateFileW(usedPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h != INVALID_HANDLE_VALUE) return h;

        BuildFallbackDumpPath(usedPath, cchPath, code, addr, tid);
        h = ::CreateFileW(usedPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        return h;
    }

    static BOOL CALLBACK DumpCallback(PVOID, const PMINIDUMP_CALLBACK_INPUT in, PMINIDUMP_CALLBACK_OUTPUT out)
    {
        if (!in || !out) return TRUE;
        switch (in->CallbackType)
        {
        case ThreadCallback:
            out->ThreadWriteFlags |= ThreadWriteInstructionWindow;
            return TRUE;
        default:
            return TRUE;
        }
    }

    static void BuildUserStreams(const EXCEPTION_POINTERS* const ep,
        OUT MINIDUMP_USER_STREAM_INFORMATION& usi,
        OUT MINIDUMP_USER_STREAM& stream,
        OUT std::unique_ptr<char[]>& owned,
        const DWORD crashTid)
    {
        wchar_t img[MAX_PATH]{}; ::GetModuleFileNameW(nullptr, img, MAX_PATH);
        const wchar_t* const cmd = ::GetCommandLineW();
        SYSTEMTIME st; ::GetLocalTime(&st);
        const DWORD pid = ::GetCurrentProcessId();
        const DWORD tid = crashTid; // 크래쉬 발생한 스레드 ID
        const DWORD code = ep ? ep->ExceptionRecord->ExceptionCode : 0xE0000001;
        const void* const addr = ep ? ep->ExceptionRecord->ExceptionAddress : nullptr;

        wchar_t text[2048]{};
        ::StringCchPrintfW(text, _countof(text),
            L"[Crash Summary]\r\n"
            L"Time         : %04u-%02u-%02u %02u:%02u:%02u\r\n"
            L"Process      : %s (pid=%u)\r\n"
            L"Crash Thread : tid=%u (0x%08X)\r\n" // 크래쉬 스레드 ID
            L"ExceptCode   : 0x%08X\r\n"
            L"ExceptAddr   : %p\r\n"
            L"CmdLine      : %s\r\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
            img, pid, tid, tid, code, addr, cmd ? cmd : L"(null)");

        const int need = ::WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
        owned.reset(new char[need > 0 ? need : 1]); // 유니크 포인터에 할당
        if (need > 0)::WideCharToMultiByte(CP_UTF8, 0, text, -1, owned.get(), need, nullptr, nullptr);

        stream.Type = CommentStreamA;
        stream.Buffer = (PVOID)owned.get();
        stream.BufferSize = (ULONG)(need > 0 ? need : 0);

        usi.UserStreamCount = 1;
        usi.UserStreamArray = &stream;
    }

    static void FreeExceptionRecordChain(EXCEPTION_RECORD* p) noexcept
    {
        while (p)
        {
            EXCEPTION_RECORD* const next = p->ExceptionRecord;
            delete p;
            p = next;
        }
    }

    using ErChainPtr = std::unique_ptr<EXCEPTION_RECORD, void(*)(EXCEPTION_RECORD*)>;

    static ErChainPtr DeepCopyExceptionRecordChain(const EXCEPTION_RECORD* const src) noexcept
    {
        ErChainPtr nullRet(nullptr, &FreeExceptionRecordChain);
        if (!src) return nullRet;

        EXCEPTION_RECORD* head = new(std::nothrow) EXCEPTION_RECORD;
        if (!head) return nullRet;

        *head = *src;
        head->ExceptionRecord = nullptr;

        EXCEPTION_RECORD* prev = head;
        const EXCEPTION_RECORD* cur = src->ExceptionRecord;

        while (cur)
        {
            EXCEPTION_RECORD* node = new(std::nothrow) EXCEPTION_RECORD;
            if (!node)
            {
                FreeExceptionRecordChain(head);
                return nullRet;
            }
            *node = *cur;
            node->ExceptionRecord = nullptr;

            prev->ExceptionRecord = node;
            prev = node;
            cur = cur->ExceptionRecord;
        }

        return ErChainPtr(head, &FreeExceptionRecordChain);
    }

    struct DumpCtx
    {
        MINIDUMP_TYPE Type;
        wchar_t Path[MAX_PATH];

        ErChainPtr                      OwnedEr{ nullptr, &FreeExceptionRecordChain };
        std::unique_ptr<CONTEXT>        OwnedCtx;
        EXCEPTION_POINTERS              LocalEp{};
        DWORD                           CrashTid = 0;
    };

    static BOOL WriteDumpNow(const HANDLE hFile,
        const EXCEPTION_POINTERS* const ep,
        const MINIDUMP_TYPE type,
        const DWORD crashTid)
    {
        MINIDUMP_EXCEPTION_INFORMATION mei{};
        MINIDUMP_EXCEPTION_INFORMATION* pMei = nullptr;
        if (ep)
        {
            // 크래쉬 스레드의 TID
            mei.ThreadId = crashTid;
            mei.ExceptionPointers = const_cast<EXCEPTION_POINTERS*>(ep);
            mei.ClientPointers = FALSE;
            pMei = &mei;
        }

        MINIDUMP_CALLBACK_INFORMATION mci{};
        mci.CallbackRoutine = DumpCallback;

        MINIDUMP_USER_STREAM_INFORMATION usi{};
        MINIDUMP_USER_STREAM stream{};
        std::unique_ptr<char[]> owned;

        BuildUserStreams(const_cast<EXCEPTION_POINTERS*>(ep), usi, stream, owned, crashTid);

        const BOOL ok = ::MiniDumpWriteDump(
            ::GetCurrentProcess(),
            ::GetCurrentProcessId(),
            hFile,
            type,
            pMei,
            &usi,
            &mci
        );

        ::FlushFileBuffers(hFile);
        ::CloseHandle(hFile);
        return ok;
    }

    static unsigned __stdcall DumpThreadProc(void* const param)
    {
        const std::unique_ptr<DumpCtx> c(reinterpret_cast<DumpCtx*>(param));

        const HANDLE h = ::CreateFileW(
            c->Path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (h != INVALID_HANDLE_VALUE)
        {
            const EXCEPTION_POINTERS* const pep =
                (c->OwnedEr && c->OwnedCtx) ? &c->LocalEp : nullptr;

            if (!WriteDumpNow(h, pep, c->Type, c->CrashTid))
            {
                const HANDLE h2 = ::CreateFileW(
                    c->Path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (h2 != INVALID_HANDLE_VALUE)
                {
                    WriteDumpNow(h2, pep, g_kFatMini, c->CrashTid);
                }
            }
        }
        return 0;
    }

    LONG WINAPI TopLevelFilter(EXCEPTION_POINTERS* const ep)
    {
        static std::atomic<bool> s_dumped{ false };

        if (s_dumped.exchange(true)) //한번만
            return EXCEPTION_EXECUTE_HANDLER;

        ULONG guar = static_cast<ULONG>(128 * 1024);
        ::SetThreadStackGuarantee(&guar);

        const DWORD code = ep ? ep->ExceptionRecord->ExceptionCode : 0xE0000001;
        const uintptr_t addr = ep ? (uintptr_t)ep->ExceptionRecord->ExceptionAddress : 0;
        const DWORD tid = ::GetCurrentThreadId(); // 크래시 스레드 ID

        wchar_t path[MAX_PATH] = {};
        const HANDLE hProbe = OpenDumpFileWithFallback(path, _countof(path), code, addr, tid);
        if (hProbe != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(hProbe);

            // stack을 피해서 할당
            DumpCtx* const ctx = new(std::nothrow) DumpCtx{};
            if (!ctx)
            {
                const HANDLE h = ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (h != INVALID_HANDLE_VALUE)
                {
                    WriteDumpNow(h, ep, g_kMaxType, tid);
                }
                return EXCEPTION_EXECUTE_HANDLER;
            }

            ctx->Type = g_kMaxType;
            ctx->CrashTid = tid;
            ::StringCchCopyW(ctx->Path, _countof(ctx->Path), path);

            if (ep)
            {
                // 깊은복사
                ctx->OwnedEr = DeepCopyExceptionRecordChain(ep->ExceptionRecord);
                ctx->OwnedCtx = std::make_unique<CONTEXT>(*ep->ContextRecord);
                if (ctx->OwnedEr && ctx->OwnedCtx)
                {
                    ctx->LocalEp.ExceptionRecord = ctx->OwnedEr.get();
                    ctx->LocalEp.ContextRecord = ctx->OwnedCtx.get();
                }
            }
            // 덤프 뱉는 스레드 따로
            const uintptr_t th = _beginthreadex(nullptr, 0, &DumpThreadProc, ctx, 0, nullptr);
            if (th)
            {
                ::WaitForSingleObject(reinterpret_cast<HANDLE>(th), 30000);
                ::CloseHandle(reinterpret_cast<HANDLE>(th));
            }
            else
            {
                const HANDLE h = ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (h != INVALID_HANDLE_VALUE)
                {
                    const EXCEPTION_POINTERS* pep =
                        (ctx->OwnedEr && ctx->OwnedCtx) ? &ctx->LocalEp : nullptr;
                    WriteDumpNow(h, pep, g_kMaxType, ctx->CrashTid);
                }
                delete ctx;
            }
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }

    static void WriteDumpWithoutException()
    {
#if defined(_M_X64) || defined(_M_IX86) || defined(_M_ARM64)
        CONTEXT ctx{}; RtlCaptureContext(&ctx);

        EXCEPTION_RECORD er{};
        er.ExceptionCode = 0xE0000001; // 커스텀
        er.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
#if defined(_M_X64)
        er.ExceptionAddress = (PVOID)(ctx.Rip);
#elif defined(_M_IX86)
        er.ExceptionAddress = (PVOID)(ctx.Eip);
#elif defined(_M_ARM64)
        er.ExceptionAddress = (PVOID)(ctx.Pc);
#endif
        EXCEPTION_POINTERS ep{ &er, &ctx };

        const DWORD code = er.ExceptionCode;
        const uintptr_t addr =
#if defined(_M_X64) || defined(_M_ARM64)
            reinterpret_cast<uintptr_t>(er.ExceptionAddress);
#else
            (uintptr_t)(er.ExceptionAddress);
#endif
        const DWORD tid = ::GetCurrentThreadId();

        wchar_t path[MAX_PATH] = {};
        const HANDLE h = OpenDumpFileWithFallback(path, _countof(path), code, addr, tid);
        if (h != INVALID_HANDLE_VALUE)
        {
            if (!WriteDumpNow(h, &ep, g_kFatMini, tid))
                LogStackTrace();
        }
#else
        LogStackTrace();
#endif
    }

    static void __cdecl InvalidParameterHandler(
        const wchar_t* expr, const wchar_t* func, const wchar_t* file, unsigned int line, uintptr_t)
    {
        (void)expr; (void)func; (void)file; (void)line;
        WriteDumpWithoutException();
        std::abort();
    }

    static void __cdecl PurecallHandler()
    {
        WriteDumpWithoutException();
        std::abort();
    }

    static void __cdecl TerminateHandler()
    {
        WriteDumpWithoutException();
        std::abort();
    }

    static void __cdecl UnexpectedHandler()
    {
        WriteDumpWithoutException();
        std::abort();
    }

    static void __cdecl NewHandler()
    {
        WriteDumpWithoutException();
        std::abort();
    }

    static void __cdecl SignalHandler(int)
    {
        WriteDumpWithoutException();
        std::abort();
    }

    void InstallCrashHandlers()
    {
        ::SetUnhandledExceptionFilter(&TopLevelFilter);

        _set_invalid_parameter_handler(&InvalidParameterHandler);
        _set_purecall_handler(&PurecallHandler);
        std::set_terminate(&TerminateHandler);
#if _HAS_UNEXPECTED
        std::set_unexpected(&UnexpectedHandler);
#endif
        std::set_new_handler(&NewHandler);

        std::signal(SIGABRT, &SignalHandler);
        std::signal(SIGSEGV, &SignalHandler);
        std::signal(SIGILL, &SignalHandler);
        std::signal(SIGFPE, &SignalHandler);
    }
}