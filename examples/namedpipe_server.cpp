// CIX C++ library
// Copyright (c) Jean-Charles Lefebvre
// SPDX-License-Identifier: MIT


namedpipe_worker::namedpipe_worker()
    : m_stop_event{nullptr}
    , m_pipe_event{nullptr}
    , m_pipe(std::make_shared<cix::win_namedpipe_server>())
{
    m_pipe_event = CreateEvent(nullptr, TRUE, FALSE, nullptr);  // manual reset
    if (!m_pipe_event)
        CIX_THROW_WINERR("failed to create pipe event");
}


namedpipe_worker::~namedpipe_worker()
{
    m_pipe->set_listener(nullptr);
    m_pipe->stop();
    m_pipe.reset();

    if (m_pipe_event)
    {
        CloseHandle(m_pipe_event);
        m_pipe_event = nullptr;
    }
}


exit_t namedpipe_worker::init(HANDLE stop_event, const std::wstring& pipe_base_name_)
{
    assert(stop_event);
    m_stop_event = stop_event;

    m_pipe_path = L"\\\\.\\pipe\\";

    if (!pipe_base_name_.empty())
    {
        m_pipe_path += pipe_base_name_;
    }
    else
    {
        std::wstring mod_path;
        std::wstring svc_name;

        if (!svc::auto_name(mod_path, svc_name) || svc_name.empty())
            return APP_EXITCODE_ERROR;

        m_pipe_path += svc_name;
    }

    return APP_EXITCODE_OK;
}


exit_t namedpipe_worker::main_loop()
{
    const HANDLE events[2] = { m_stop_event, m_pipe_event };

    assert(m_stop_event);
    assert(m_pipe_event);

    // m_pipe->set_flags(cix::win_namedpipe_server::flag_accept_remote);
    m_pipe->set_path(m_pipe_path);
    m_pipe->set_listener(this->shared_from_this());

    m_pipe->launch();

    for (;;)
    {
        const auto wait_res = WaitForMultipleObjects(
            static_cast<DWORD>(cix::countof(events)),
            reinterpret_cast<const HANDLE*>(&events),
            FALSE, INFINITE);

        if (wait_res == WAIT_OBJECT_0 + 0)  // stop event
        {
            break;
        }
        else if (wait_res == WAIT_OBJECT_0 + 1)  // pipe event
        {
            ResetEvent(m_pipe_event);
            this->process_pipe_event();
        }
        else
        {
            LOGERROR(
                "worker failed to enter in waiting mode (result {}; error {})",
                wait_res, GetLastError());
            return APP_EXITCODE_API;
        }
    }

    LOGTRACE("stopping namedpipe_server");
    m_pipe->set_listener(nullptr);
    m_pipe->stop();
    LOGTRACE("namedpipe_server stopped");

    return APP_EXITCODE_OK;
}


void namedpipe_worker::on_namedpipe_connected(
    std::shared_ptr<cix::win_namedpipe_server> pipe,
    cix::win_namedpipe_server::instance_token_t pipe_instance_token)
{
    CIX_UNVAR(pipe);
    CIX_UNVAR(pipe_instance_token);

    LOGTRACE("pipe client connected: {}", pipe_instance_token);
}


void namedpipe_worker::on_namedpipe_recv(
    std::shared_ptr<cix::win_namedpipe_server> pipe,
    cix::win_namedpipe_server::instance_token_t pipe_instance_token,
    cix::win_namedpipe_server::packet_t&& packet)
{
    assert(pipe);
    assert(m_pipe);
    assert(pipe == m_pipe);
    assert(!packet.empty());

#ifdef _DEBUG
    std::basic_string_view<char> view((const char*)packet.data(), packet.size());
    LOGTRACE("PIPE RECV {} bytes of data: {}", packet.size(), view);
#endif

    std::scoped_lock lock(m_mutex);

    auto instance = this->find_or_create_pipe_instance(pipe_instance_token);

    if (instance->input_buffer.empty())
    {
        instance->input_buffer.swap(packet);
    }
    else
    {
        std::copy(
            packet.begin(), packet.end(),
            std::back_inserter(instance->input_buffer));
    }

    auto it = std::find(
        m_received_data.begin(), m_received_data.end(),
        pipe_instance_token);
    if (it == m_received_data.end())
        m_received_data.push_back(pipe_instance_token);

    SetEvent(m_pipe_event);
}


void namedpipe_worker::on_namedpipe_closed(
    std::shared_ptr<cix::win_namedpipe_server> pipe,
    cix::win_namedpipe_server::instance_token_t pipe_instance_token)
{
#ifdef _DEBUG
    std::scoped_lock lock(m_mutex);  // for the LOGTRACE call

    const auto remaining =
        m_pipe_instances.empty() ? 0 : m_pipe_instances.size() - 1;

    LOGTRACE(
        "pipe client DISCONNECTED #{} ({} remaining)",
        pipe_instance_token, remaining);
#endif

    this->erase_pipe_instance(pipe_instance_token);
}


void namedpipe_worker::process_pipe_event()
{
    cix::lock_guard lock(m_mutex, std::defer_lock);  // not locked yet
    std::shared_ptr<pipe_instance_t> pipe_instance;

    for (;;)
    {
        lock.lock();

        if (!m_received_data.empty())
        {
            // CAUTION: do not use *auto* here as it would translate to a
            // reference. We want *token* to be copied locally before being
            // erased from the queue.
            cix::win_namedpipe_server::instance_token_t token =
                m_received_data.front();

            m_received_data.pop_front();

            auto it = m_pipe_instances.find(token);
            if (it != m_pipe_instances.end())
                pipe_instance = it->second;
        }

        lock.unlock();

        if (!pipe_instance)
            break;

        this->process_instance_input(pipe_instance);

        pipe_instance.reset();
    }
}


void namedpipe_worker::process_instance_input(
    std::shared_ptr<pipe_instance_t> pipe_instance)
{
    cix::lock_guard lock(m_mutex, std::defer_lock);  // not locked yet

    for (;;)
    {
        lock.lock();

        if (pipe_instance->input_buffer.empty())
        {
            lock.unlock();
            return;
        }

        // TODO: here, fully or partially consume the content of
        // pipe_instance->input_buffer as quickly as possible so the lock can be
        // released
        //
        // Example:
        //
        //   auto packet = extract_next_packet(pipe_instance->input_buffer);

        lock.unlock();

        // TODO: here, do something with the extracted packet
        //
        // Example:
        //
        //   this->process_input_packet(pipe_instance, packet)
    }
}


// void namedpipe_worker::process_input_packet(
//     std::shared_ptr<pipe_instance_t> pipe_instance,
//     const packet_t& packet)
// {
//     // TODO: here, do something with the packet
//     //
//     // Example:
//     //
//     //   // and echo back
//     //   m_pipe->push_packet(pipe_instance->token, std::move(packet));
// }


std::shared_ptr<namedpipe_worker::pipe_instance_t>
namedpipe_worker::find_or_create_pipe_instance(
    cix::win_namedpipe_server::instance_token_t token)
{
    std::scoped_lock lock(m_mutex);

    auto it = m_pipe_instances.find(token);

    if (it != m_pipe_instances.end())
    {
        return it->second;
    }
    else
    {
        const auto res = m_pipe_instances.insert(
            std::make_pair(token, std::make_shared<pipe_instance_t>(token)));

        LOGTRACE(
            "pipe client inserted #{} (count: {})",
            token, m_pipe_instances.size());

        return res.first->second;
    }
}


void namedpipe_worker::erase_pipe_instance(
    cix::win_namedpipe_server::instance_token_t token)
{
    std::scoped_lock lock(m_mutex);

    m_pipe_instances.erase(token);

    auto it = std::find(m_received_data.begin(), m_received_data.end(), token);
    if (it != m_received_data.end())
        m_received_data.erase(it);
}
