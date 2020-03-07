// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once

#include "detail/ensure_cix.h"

#if defined(CIX_ENABLE_WIN_NAMEDPIPE_SERVER) && (CIX_PLATFORM == CIX_PLATFORM_WINDOWS)

namespace cix {

// a threaded server-side of a named pipe
// it manages multiple instances - clients - with a single dedicated thread by
// using completion routines
class win_namedpipe_server :
    public std::enable_shared_from_this<win_namedpipe_server>
{
public:
    enum : DWORD { io_buffer_default_size = 8 * 1024 };  // xkcd221

    enum flags_t : unsigned
    {
        flag_message       = 0x01,  // defaults to byte mode
        flag_accept_remote = 0x02,
        flag_impersonate   = 0x04,  // null dacl

        flag_default = 0,
    };

    typedef cix::best_fit<HANDLE>::unsigned_t instance_token_t;

    typedef std::vector<std::uint8_t> bytes_t;

    struct listener_t
    {
        virtual void on_namedpipe_connected(
            std::shared_ptr<win_namedpipe_server> pipe,
            win_namedpipe_server::instance_token_t pipe_instance_token) = 0;

        virtual void on_namedpipe_recv(
            std::shared_ptr<win_namedpipe_server> pipe,
            win_namedpipe_server::instance_token_t pipe_instance_token,
            win_namedpipe_server::bytes_t&& packet) = 0;

        virtual void on_namedpipe_closed(
            std::shared_ptr<win_namedpipe_server> pipe,
            win_namedpipe_server::instance_token_t pipe_instance_token) = 0;
    };

private:
    class instance_t : public std::enable_shared_from_this<instance_t>
    {
    public:
        struct overlapped_t
        {
            overlapped_t() = delete;
            overlapped_t(
                    std::shared_ptr<instance_t> instance_,
                    bytes_t&& packet_)
                : ol{}
                , instance(instance_)
                , packet(std::move(packet_))
                { }
            ~overlapped_t() = default;

            OVERLAPPED ol;  // CAUTION: must remain first
            std::weak_ptr<instance_t> instance;
            bytes_t packet;  // input or output data
        };

    public:
        instance_t() = delete;
        instance_t(
            std::shared_ptr<win_namedpipe_server> parent,
            HANDLE pipe);
        ~instance_t();

        instance_token_t token() const;
        std::shared_ptr<win_namedpipe_server> parent() const;
        bool orphan() const;

        void proceed();
        bool send(bytes_t&& packet);
        void disconnect();
        void close();

        void on_recv(std::shared_ptr<overlapped_t> ol);
        void on_written();

    private:
        // properties
        mutable std::recursive_mutex m_mutex;
        std::weak_ptr<win_namedpipe_server> m_parent;
        instance_token_t m_token;
        HANDLE m_pipe;

        // state
        std::shared_ptr<overlapped_t> m_olread;
        std::shared_ptr<overlapped_t> m_olwrite;
        std::queue<bytes_t> m_output;
    };

public:
    win_namedpipe_server();
    ~win_namedpipe_server();

    void set_flags(flags_t flags);
    void set_path(const std::wstring& pipe_path);
    void set_listener(std::shared_ptr<listener_t> listener);  // can be null

    void launch();

    bool send(instance_token_t instance_token, bytes_t&& packet);
    bool send_to_first(bytes_t&& packet);
    std::size_t broadcast_packet(bytes_t&& packet);

    bool disconnect_instance(instance_token_t instance_token);

    void stop();

private:
    void maintenance_thread();
    HANDLE open_and_listen(OVERLAPPED* ol, bool* out_connecting);
    void create_instance(HANDLE pipe_handle);
    void handle_proceed_event();

    void notify_recv(instance_token_t token, bytes_t&& packet);
    void notify_closed(instance_token_t token);

    static void WINAPI apc_completed_read(
        DWORD error, DWORD bytes_read, LPOVERLAPPED ol);
    static void WINAPI apc_completed_write(
        DWORD error, DWORD bytes_written, LPOVERLAPPED ol);

private:
    mutable std::recursive_mutex m_mutex;
    std::wstring m_path;
    std::weak_ptr<listener_t> m_listener;
    std::unique_ptr<std::thread> m_thread;
    HANDLE m_stop_event;
    HANDLE m_connect_event;
    HANDLE m_proceed_event;
    flags_t m_flags;
    std::map<instance_token_t, std::shared_ptr<instance_t>> m_instances;
    std::set<instance_token_t> m_proceed;

private:
    static std::mutex ms_overlapped_registry_mutex;
    static std::map<
        instance_t::overlapped_t*,
        std::shared_ptr<instance_t::overlapped_t>>
            ms_overlapped_registry;
};

CIX_IMPLEMENT_ENUM_BITOPS(win_namedpipe_server::flags_t)

}  // namespace cix

#endif  // #if defined(CIX_ENABLE_WIN_NAMEDPIPE_SERVER) && (CIX_PLATFORM == CIX_PLATFORM_WINDOWS)
