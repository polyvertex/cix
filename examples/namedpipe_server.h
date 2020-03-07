// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT

#pragma once


class namedpipe_worker :
    public cix::win_namedpipe_server::listener_t,
    public std::enable_shared_from_this<namedpipe_worker>
{
public:
    enum : std::size_t { max_clients = MAXIMUM_WAIT_OBJECTS - 1 };

private:
    struct pipe_instance_t
    {
        pipe_instance_t() = delete;
        pipe_instance_t(cix::win_namedpipe_server::instance_token_t token_)
            : token{token_}
            { }

        cix::win_namedpipe_server::instance_token_t token;
        cix::win_namedpipe_server::packet_t input_buffer;
    };

public:
    namedpipe_worker();
    ~namedpipe_worker();

    exit_t init(HANDLE stop_event, const std::wstring& pipe_base_name);
    exit_t main_loop();

private:
    // from cix::win_namedpipe_server::listener_t
    void on_namedpipe_connected(
        std::shared_ptr<cix::win_namedpipe_server> pipe,
        cix::win_namedpipe_server::instance_token_t pipe_instance_token);
    void on_namedpipe_recv(
        std::shared_ptr<cix::win_namedpipe_server> pipe,
        cix::win_namedpipe_server::instance_token_t pipe_instance_token,
        cix::win_namedpipe_server::packet_t&& packet);
    void on_namedpipe_closed(
        std::shared_ptr<cix::win_namedpipe_server> pipe,
        cix::win_namedpipe_server::instance_token_t pipe_instance_token);

    void process_pipe_event();
    void process_instance_input(
        std::shared_ptr<pipe_instance_t> pipe_instance);
    // void process_input_packet(
    //     std::shared_ptr<pipe_instance_t> pipe_instance,
    //     const packet_t& packet);

    std::shared_ptr<pipe_instance_t> find_or_create_pipe_instance(
        cix::win_namedpipe_server::instance_token_t token);
    void erase_pipe_instance(cix::win_namedpipe_server::instance_token_t token);

private:
    std::recursive_mutex m_mutex;

    HANDLE m_stop_event;
    HANDLE m_pipe_event;
    std::wstring m_pipe_path;
    std::shared_ptr<cix::win_namedpipe_server> m_pipe;

    std::map<
        cix::win_namedpipe_server::instance_token_t,
        std::shared_ptr<pipe_instance_t>>
            m_pipe_instances;

    std::deque<cix::win_namedpipe_server::instance_token_t> m_received_data;
};
