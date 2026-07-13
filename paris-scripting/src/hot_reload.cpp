#include "paris/hot_reload.hpp"

#include "paris/engine.hpp"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace paris {

struct HotReload::Impl {
    ScriptEngine&               engine;
    std::thread                 worker;
    std::atomic<bool>           running{ false };
    std::filesystem::path       dir;
    bool                        recursive = false;
    std::function<void(const std::filesystem::path&)> on_change;

    // Debounce: some editors write in two steps (delete + create) and fire
    // multiple change events per save. Coalesce anything under 200 ms.
    std::mutex                                    dedup_mtx;
    std::unordered_map<std::string,
        std::chrono::steady_clock::time_point>    last_seen;

    explicit Impl(ScriptEngine& e) : engine(e) {}

#ifdef _WIN32
    void run() {
        HANDLE h = CreateFileW(dir.wstring().c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr, OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            log_console_error("hot_reload: cannot open watch dir");
            running = false;
            return;
        }

        OVERLAPPED ov{};
        ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        alignas(DWORD) BYTE buf[8192];

        while (running.load()) {
            DWORD returned = 0;
            ResetEvent(ov.hEvent);
            if (!ReadDirectoryChangesW(h, buf, sizeof(buf), recursive,
                    FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE |
                    FILE_NOTIFY_CHANGE_SIZE,
                    &returned, &ov, nullptr)) {
                log_console_error("hot_reload: ReadDirectoryChangesW failed");
                break;
            }

            // Wait, but poll `running` so stop() can wake us.
            while (running.load()) {
                DWORD wait = WaitForSingleObject(ov.hEvent, 250);
                if (wait == WAIT_OBJECT_0) break;
                if (wait != WAIT_TIMEOUT) { running = false; break; }
            }
            if (!running.load()) break;

            DWORD bytes = 0;
            if (!GetOverlappedResult(h, &ov, &bytes, FALSE) || bytes == 0)
                continue;

            auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buf);
            for (;;) {
                std::wstring name(info->FileName,
                                  info->FileNameLength / sizeof(WCHAR));
                std::filesystem::path p = dir / std::filesystem::path(name);
                auto ext = p.extension().string();
                if (ext == ".lua" || ext == ".as") maybe_fire(p);

                if (!info->NextEntryOffset) break;
                info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
            }
        }

        CloseHandle(ov.hEvent);
        CloseHandle(h);
    }
#else
    void run() {
        log_console("hot_reload: no watcher backend on this platform");
        running = false;
    }
#endif

    void maybe_fire(const std::filesystem::path& p) {
        auto key = p.string();
        auto now = std::chrono::steady_clock::now();
        {
            std::lock_guard lk(dedup_mtx);
            auto it = last_seen.find(key);
            if (it != last_seen.end()) {
                auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
                if (dt < 200) return;
            }
            last_seen[key] = now;
        }
        if (on_change) on_change(p);
        else           engine.load_file(p);
    }
};

HotReload::HotReload(ScriptEngine& e) : impl_(std::make_unique<Impl>(e)) {}
HotReload::~HotReload() { stop(); }

void HotReload::start(const std::filesystem::path& dir, bool recursive) {
    if (impl_->running.load()) return;
    impl_->dir       = dir;
    impl_->recursive = recursive;
    impl_->running   = true;
    impl_->worker    = std::thread([this]{ impl_->run(); });
}

void HotReload::stop() {
    if (!impl_->running.exchange(false)) return;
    if (impl_->worker.joinable()) impl_->worker.join();
}

bool HotReload::is_running() const { return impl_->running.load(); }

void HotReload::set_on_change(std::function<void(const std::filesystem::path&)> fn) {
    impl_->on_change = std::move(fn);
}

} // namespace paris
