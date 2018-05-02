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
 *     What is this file about?
 *
 * Revision history:
 *     xxxx-xx-xx, author, first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */


# include "emulator.h"
# include "scheduler.h"
# include "env.sim.h"

# ifdef __TITLE__
# undef __TITLE__
# endif
# define __TITLE__ "tools.emulator"

namespace dsn { namespace tools {

void emulator::install(service_spec& spec)
{   
    scheduler::instance();

    if (spec.aio_factory_name == "")
        spec.aio_factory_name = ("dsn::tools::sim_aio_provider");

    if (spec.env_factory_name == "")
        spec.env_factory_name = ("dsn::tools::sim_env_provider");

    if (spec.timer_factory_name == "")
        spec.timer_factory_name = ("dsn::tools::sim_timer_service");

    network_client_config cs;
    cs.factory_name = "dsn::tools::sim_network_provider";
    cs.message_buffer_block_size = 1024 * 64;
    spec.network_default_client_cfs[NET_CHANNEL_TCP] = cs;

    network_server_config cs2;
    cs2.port = 0;
    cs2.factory_name = "dsn::tools::sim_network_provider";
    cs2.message_buffer_block_size = 1024 * 64;
    cs2.channel = NET_CHANNEL_TCP;
    spec.network_default_server_cfs[cs2] = cs2;

    if (spec.perf_counter_factory_name == "")
        spec.perf_counter_factory_name = "dsn::tools::simple_perf_counter";
    
    if (spec.logging_factory_name == "")
        spec.logging_factory_name = "dsn::tools::simple_logger";

    if (spec.memory_factory_name == "")
        spec.memory_factory_name = "dsn::default_memory_provider";

    if (spec.tools_memory_factory_name == "")
        spec.tools_memory_factory_name = "dsn::default_memory_provider";

    if (spec.lock_factory_name == "")
        spec.lock_factory_name = ("dsn::tools::sim_lock_provider");

    if (spec.lock_nr_factory_name == "")
        spec.lock_nr_factory_name = ("dsn::tools::sim_lock_nr_provider");

    if (spec.rwlock_nr_factory_name == "")
        spec.rwlock_nr_factory_name = ("dsn::tools::sim_rwlock_nr_provider");

    if (spec.semaphore_factory_name == "")
        spec.semaphore_factory_name = ("dsn::tools::sim_semaphore_provider");

    if (spec.nfs_factory_name == "")
        spec.nfs_factory_name = "dsn::service::nfs_node_simple";

    for (auto it = spec.threadpool_specs.begin(); it != spec.threadpool_specs.end(); ++it)
    {
        threadpool_spec& tspec = *it;

        if (tspec.worker_factory_name == "")
            tspec.worker_factory_name = ("dsn::task_worker");

        if (tspec.queue_factory_name == "")
            tspec.queue_factory_name = ("dsn::tools::sim_task_queue");
    }

    sys_init_after_app_created.put_back(
        emulator::on_system_init_for_add_global_checker,
        "checkers.install"
    );

    sys_exit.put_front(emulator::on_system_exit, "emulator");
}

void emulator::on_system_init_for_add_global_checker()
{
    safe_vector<global_checker> checkers;
    ::dsn::get_registered_checkers(checkers);

    auto t = dynamic_cast<dsn::tools::emulator*>(::dsn::tools::get_current_tool());
    if (t != nullptr)
    {
        for (auto& c : checkers)
        {
            t->add_checker(c.name.c_str(), c.create, c.apply);
        }
    }
}

void emulator::on_system_exit(sys_exit_type st)
{
    derror("system exits, you can replay this process using random seed %d",        
        sim_env_provider::seed()
        );
}

void emulator::add_checker(const char* name, dsn_checker_create create, dsn_checker_apply apply)
{
    scheduler::instance().add_checker(name, create, apply);
}

void emulator::run()
{
    scheduler::instance().start();
    tool_app::run();
}

}} // end namespace dsn::tools
