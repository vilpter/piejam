// PieJam - An audio mixer for Raspberry Pi.
// SPDX-FileCopyrightText: 2020-2025  Dimitrij Kotrev
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <piejam/thread/configuration.h>

#include <concepts>
#include <functional>
#include <semaphore>
#include <thread>

namespace piejam::thread
{

//! Single task worker thread for real-time tasks.
//!
//! Constraints:
//!   - Only stateless callables or std::ref(...) are allowed as tasks.
//!     Passing capturing lambdas or heavy functors may allocate and
//!     break real-time safety.
//!   - Tasks must not throw exceptions. Any uncaught exception may
//!     terminate the program.
class worker
{
public:
    using task_t = std::function<void()>;

    worker(thread::configuration conf = {})
        : m_thread([this, conf = std::move(conf)](std::stop_token stoken) {
            conf.apply();

            while (true)
            {
                m_sem_work.acquire();

                if (stoken.stop_requested())
                {
                    break;
                }

                m_task();

                m_sem_finished.release();
            }
        })
    {
    }

    worker(worker const&) = delete;
    worker(worker&&) = delete;

    ~worker()
    {
        // Acquire finished to ensure no task is in progress (blocks until any
        // running task completes).
        m_sem_finished.acquire();

        // Request cooperative stop and wake thread so it can observe stop and
        // exit.
        m_thread.request_stop();
        m_sem_work.release();

        // jthread destructor will join the thread automatically when m_thread
        // is destroyed after this destructor finishes.
    }

    auto operator=(worker const&) -> worker& = delete;
    auto operator=(worker&&) -> worker& = delete;

    //! Schedule a callable to run on the worker. This blocks until the worker
    //! is ready to accept a new task.
    template <std::invocable<> F>
    void wakeup(F&& task) noexcept
    {
        // Acquire finished semaphore to ensure exclusive access (wait until
        // previous work done).
        m_sem_finished.acquire();

        m_task = std::forward<F>(task);

        // Notify worker thread that work is available.
        m_sem_work.release();
    }

    //! Block until the worker has finished its current task (if any).
    void wait() noexcept
    {
        m_sem_finished.acquire();
        m_sem_finished.release();
    }

private:
    std::binary_semaphore m_sem_work{0};
    std::binary_semaphore m_sem_finished{1};

    task_t m_task{[]() {}};
    std::jthread m_thread;
};

} // namespace piejam::thread
