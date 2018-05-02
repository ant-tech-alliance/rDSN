/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Microsoft Corporation
 *
 * -=- Robust Distributed System Nucleus (rDSN) -=-
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * Description:
 *     Unit-test for sim lock.
 *
 * Revision history:
 *     Nov., 2015, @xiaotz (Xiaotong Zhang), first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */

# include <dsn/service_api_c.h>
# include <dsn/tool_api.h>
# include <dsn/tool-api/task.h>
# include <dsn/cpp/auto_codes.h>
# include <dsn/utility/misc.h>
# include <dsn/utility/synchronize.h>
# include <gtest/gtest.h>
# include <thread>

# include "task_engine.sim.h"
# include "scheduler.h"

TEST(tools_emulator, dsn_semaphore)
{
    if(dsn::task::get_current_worker() == nullptr)
        return;

    dsn_handle_t s = dsn_semaphore_create(2);
    dsn_semaphore_wait(s);
    ASSERT_TRUE(dsn_semaphore_wait_timeout(s, 10));
    ASSERT_FALSE(dsn_semaphore_wait_timeout(s, 0));
    dsn_semaphore_signal(s, 1);
    dsn_semaphore_wait(s);
    dsn_semaphore_destroy(s);
}

TEST(tools_emulator, dsn_lock_nr)
{
    if(dsn::task::get_current_worker() == nullptr)
        return;
    
    if (dsn::tools::get_current_tool()->name() != "emulator")
        return;

    dsn::tools::sim_lock_nr_provider* s = new dsn::tools::sim_lock_nr_provider(nullptr);
    s->lock();
    s->unlock();
    EXPECT_TRUE(s->try_lock());
    s->unlock();
    delete s;
}


TEST(tools_emulator, dsn_lock)
{
    if(dsn::task::get_current_worker() == nullptr)
        return;

    if (dsn::tools::get_current_tool()->name() != "emulator")
        return;

    dsn::tools::sim_lock_provider* s = new dsn::tools::sim_lock_provider(nullptr);
    s->lock();
    EXPECT_TRUE(s->try_lock());
    s->unlock();
    s->unlock();
    delete s;
}

namespace dsn{ namespace test{
typedef std::function<void()> system_callback;
}}
TEST(tools_emulator, scheduler)
{
    if(dsn::task::get_current_worker() == nullptr)
        return;

    if (dsn::tools::get_current_tool()->name() != "emulator")
        return;

     dsn::tools::sim_worker_state* s = dsn::tools::scheduler::task_worker_ext::get(dsn::task::get_current_worker());
    dsn::utils::notify_event* evt = new dsn::utils::notify_event();
    dsn::test::system_callback callback = [evt, s](
            void)
    {
        evt->notify();
        s->is_continuation_ready = true;
        return;
    };
    dsn::tools::scheduler::instance().add_system_event(100, callback);
    dsn::tools::scheduler::instance().wait_schedule(true,false);
    evt->wait();
}
