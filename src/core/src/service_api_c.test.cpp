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
 *     Unit-test for c service api.
 *
 * Revision history:
 *     Nov., 2015, @qinzuoyan (Zuoyan Qin), first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */

# include <dsn/service_api_cpp.h>
# include <dsn/tool_api.h>
# include <gtest/gtest.h>
# include <thread>
# include "service_engine.h"
# include <dsn/cpp/test_utils.h>

using namespace dsn;

TEST(core, dsn_error)
{
    ASSERT_EQ(ERR_OK, dsn_error_register("ERR_OK"));
    ASSERT_STREQ("ERR_OK", dsn_error_to_string(ERR_OK));
}

DEFINE_THREAD_POOL_CODE(THREAD_POOL_FOR_TEST)
TEST(core, dsn_threadpool_code)
{
    ASSERT_EQ(THREAD_POOL_INVALID, dsn_threadpool_code_from_string("THREAD_POOL_NOT_EXIST", THREAD_POOL_INVALID));

    ASSERT_STREQ("THREAD_POOL_DEFAULT", dsn_threadpool_code_to_string(THREAD_POOL_DEFAULT));
    ASSERT_EQ(THREAD_POOL_DEFAULT, dsn_threadpool_code_from_string("THREAD_POOL_DEFAULT", THREAD_POOL_INVALID));
    ASSERT_LE(THREAD_POOL_DEFAULT, dsn_threadpool_code_max());

    ASSERT_STREQ("THREAD_POOL_FOR_TEST", dsn_threadpool_code_to_string(THREAD_POOL_FOR_TEST));
    ASSERT_EQ(THREAD_POOL_FOR_TEST, dsn_threadpool_code_from_string("THREAD_POOL_FOR_TEST", THREAD_POOL_INVALID));
    ASSERT_LE(THREAD_POOL_FOR_TEST, dsn_threadpool_code_max());

    ASSERT_LT(0, dsn_threadpool_get_current_tid());
}

DEFINE_TASK_CODE(TASK_CODE_COMPUTE_FOR_TEST, TASK_PRIORITY_HIGH, THREAD_POOL_DEFAULT)
DEFINE_TASK_CODE_AIO(TASK_CODE_AIO_FOR_TEST, TASK_PRIORITY_COMMON, THREAD_POOL_DEFAULT)
DEFINE_TASK_CODE_RPC(TASK_CODE_RPC_FOR_TEST, TASK_PRIORITY_LOW, THREAD_POOL_DEFAULT)
TEST(core, dsn_task_code)
{
    dsn_task_type_t type;
    dsn_task_priority_t pri;
    dsn_threadpool_code_t pool;

    ASSERT_EQ(TASK_CODE_INVALID, dsn_task_code_from_string("TASK_CODE_NOT_EXIST", TASK_CODE_INVALID));

    ASSERT_STREQ("TASK_TYPE_COMPUTE", dsn_task_type_to_string(TASK_TYPE_COMPUTE));

    ASSERT_STREQ("TASK_PRIORITY_HIGH", dsn_task_priority_to_string(TASK_PRIORITY_HIGH));

    ASSERT_STREQ("TASK_CODE_COMPUTE_FOR_TEST", dsn_task_code_to_string(TASK_CODE_COMPUTE_FOR_TEST));
    ASSERT_EQ(TASK_CODE_COMPUTE_FOR_TEST, dsn_task_code_from_string("TASK_CODE_COMPUTE_FOR_TEST", TASK_CODE_INVALID));
    ASSERT_LE(TASK_CODE_COMPUTE_FOR_TEST, dsn_task_code_max());
    dsn_task_code_query(TASK_CODE_COMPUTE_FOR_TEST, &type, &pri, &pool);
    ASSERT_EQ(TASK_TYPE_COMPUTE, type);
    ASSERT_EQ(TASK_PRIORITY_HIGH, pri);
    ASSERT_EQ(THREAD_POOL_DEFAULT, pool);

    ASSERT_STREQ("TASK_CODE_AIO_FOR_TEST", dsn_task_code_to_string(TASK_CODE_AIO_FOR_TEST));
    ASSERT_EQ(TASK_CODE_AIO_FOR_TEST, dsn_task_code_from_string("TASK_CODE_AIO_FOR_TEST", TASK_CODE_INVALID));
    ASSERT_LE(TASK_CODE_AIO_FOR_TEST, dsn_task_code_max());
    dsn_task_code_query(TASK_CODE_AIO_FOR_TEST, &type, &pri, &pool);
    ASSERT_EQ(TASK_TYPE_AIO, type);
    ASSERT_EQ(TASK_PRIORITY_COMMON, pri);
    ASSERT_EQ(THREAD_POOL_DEFAULT, pool);

    ASSERT_STREQ("TASK_CODE_RPC_FOR_TEST", dsn_task_code_to_string(TASK_CODE_RPC_FOR_TEST));
    ASSERT_EQ(TASK_CODE_RPC_FOR_TEST, dsn_task_code_from_string("TASK_CODE_RPC_FOR_TEST", TASK_CODE_INVALID));
    ASSERT_LE(TASK_CODE_RPC_FOR_TEST, dsn_task_code_max());
    dsn_task_code_query(TASK_CODE_RPC_FOR_TEST, &type, &pri, &pool);
    ASSERT_EQ(TASK_TYPE_RPC_REQUEST, type);
    ASSERT_EQ(TASK_PRIORITY_LOW, pri);
    ASSERT_EQ(THREAD_POOL_DEFAULT, pool);

    ASSERT_STREQ("TASK_CODE_RPC_FOR_TEST_ACK", dsn_task_code_to_string(TASK_CODE_RPC_FOR_TEST_ACK));
    ASSERT_EQ(TASK_CODE_RPC_FOR_TEST_ACK, dsn_task_code_from_string("TASK_CODE_RPC_FOR_TEST_ACK", TASK_CODE_INVALID));
    ASSERT_LE(TASK_CODE_RPC_FOR_TEST_ACK, dsn_task_code_max());
    dsn_task_code_query(TASK_CODE_RPC_FOR_TEST_ACK, &type, &pri, &pool);
    ASSERT_EQ(TASK_TYPE_RPC_RESPONSE, type);
    ASSERT_EQ(TASK_PRIORITY_LOW, pri);
    ASSERT_EQ(THREAD_POOL_DEFAULT, pool);

    dsn_task_code_set_threadpool(TASK_CODE_COMPUTE_FOR_TEST, THREAD_POOL_FOR_TEST);
    dsn_task_code_set_priority(TASK_CODE_COMPUTE_FOR_TEST, TASK_PRIORITY_COMMON);
    dsn_task_code_query(TASK_CODE_COMPUTE_FOR_TEST, &type, &pri, &pool);
    ASSERT_EQ(TASK_TYPE_COMPUTE, type);
    ASSERT_EQ(TASK_PRIORITY_COMMON, pri);
    ASSERT_EQ(THREAD_POOL_FOR_TEST, pool);

    dsn_task_code_set_threadpool(TASK_CODE_COMPUTE_FOR_TEST, THREAD_POOL_DEFAULT);
    dsn_task_code_set_priority(TASK_CODE_COMPUTE_FOR_TEST, TASK_PRIORITY_HIGH);
}

TEST(core, dsn_config)
{
    ASSERT_TRUE(dsn_config_get_value_bool("apps.client", "run", false, "client run"));
    ASSERT_EQ(1u, dsn_config_get_value_uint64("apps.client", "count", 100, "client count"));
    ASSERT_EQ(1.0, dsn_config_get_value_double("apps.client", "count", 100.0, "client count"));
    ASSERT_EQ(1.0, dsn_config_get_value_double("apps.client", "count", 100.0, "client count"));
    const char* buffers[100];
    int buffer_count = 100;
    ASSERT_EQ(2, dsn_config_get_all_keys("core.test", buffers, &buffer_count));
    ASSERT_EQ(2, buffer_count);
    ASSERT_STREQ("count", buffers[0]);
    ASSERT_STREQ("run", buffers[1]);
    buffer_count = 1;
    ASSERT_EQ(2, dsn_config_get_all_keys("core.test", buffers, &buffer_count));
    ASSERT_EQ(1, buffer_count);
    ASSERT_STREQ("count", buffers[0]);
}

TEST(core, dsn_coredump)
{
}

TEST(core, dsn_crc32)
{
}

TEST(core, dsn_task)
{
}

TEST(core, dsn_exlock)
{
    if(dsn::service_engine::fast_instance().spec().semaphore_factory_name == "dsn::tools::sim_semaphore_provider")
        return;
    {
        dsn_handle_t l = dsn_exlock_create(false);
        ASSERT_NE(nullptr, l);
        ASSERT_TRUE(dsn_exlock_try_lock(l));
        dsn_exlock_unlock(l);
        dsn_exlock_lock(l);
        dsn_exlock_unlock(l);
        dsn_exlock_destroy(l);
    }
    {
        dsn_handle_t l = dsn_exlock_create(true);
        ASSERT_NE(nullptr, l);
        ASSERT_TRUE(dsn_exlock_try_lock(l));
        ASSERT_TRUE(dsn_exlock_try_lock(l));
        dsn_exlock_unlock(l);
        dsn_exlock_unlock(l);
        dsn_exlock_lock(l);
        dsn_exlock_lock(l);
        dsn_exlock_unlock(l);
        dsn_exlock_unlock(l);
        dsn_exlock_destroy(l);
    }
}

TEST(core, dsn_rwlock)
{
    if(dsn::service_engine::fast_instance().spec().semaphore_factory_name == "dsn::tools::sim_semaphore_provider")
        return;
    dsn_handle_t l = dsn_rwlock_nr_create();
    ASSERT_NE(nullptr, l);
    dsn_rwlock_nr_lock_read(l);
    dsn_rwlock_nr_unlock_read(l);
    dsn_rwlock_nr_lock_write(l);
    dsn_rwlock_nr_unlock_write(l);
    dsn_rwlock_nr_destroy(l);
}

TEST(core, dsn_semaphore)
{
    if(dsn::service_engine::fast_instance().spec().semaphore_factory_name == "dsn::tools::sim_semaphore_provider")
        return;
    dsn_handle_t s = dsn_semaphore_create(2);
    dsn_semaphore_wait(s);
    ASSERT_TRUE(dsn_semaphore_wait_timeout(s, 10));
    ASSERT_FALSE(dsn_semaphore_wait_timeout(s, 10));
    dsn_semaphore_signal(s, 1);
    dsn_semaphore_wait(s);
    dsn_semaphore_destroy(s);
}

TEST(core, dsn_rpc)
{
}

struct aio_result
{
    dsn_error_t err;
    size_t sz;
    zevent evt;
};

TEST(core, dsn_file)
{
# ifndef __APPLE__
    // if in dsn_mimic_app() and disk_io_mode == IOE_PER_QUEUE
    if (task::get_current_disk() == nullptr) return;

    int64_t fin_size, fout_size;
    ASSERT_TRUE(utils::filesystem::file_size("command.txt", fin_size));
    ASSERT_LT(0, fin_size);

    dsn_handle_t fin = dsn_file_open("command.txt", O_RDONLY, 0);
    ASSERT_NE(nullptr, fin);
    dsn_handle_t fout = dsn_file_open("command.copy.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
    ASSERT_NE(nullptr, fout);
    char buffer[1024];
    uint64_t offset = 0;
    while (true)
    {
        aio_result rin;
        dsn_file_read(fin, buffer, 1024, offset, LPC_AIO_TEST_READ,
            [](dsn_error_t err, size_t sz, void* param)
            {
                aio_result* r = (aio_result*)param;
                r->err = err;
                r->sz = sz;
                r->evt.set();
            },
            &rin, 0);

        rin.evt.wait();

        if (rin.err != ERR_OK)
        {
            ASSERT_EQ(ERR_HANDLE_EOF, rin.err);
            break;
        }
        ASSERT_LT(0u, rin.sz);

        aio_result rout;
        dsn_file_write(fout, buffer, rin.sz, offset, LPC_AIO_TEST_WRITE,
            [](dsn_error_t err, size_t sz, void* param)
            {
                aio_result* r = (aio_result*)param;
                r->err = err;
                r->sz = sz;
                r->evt.set();
            },
            &rout, 0);
        rout.evt.wait();

        ASSERT_EQ(ERR_OK, rout.err);
        ASSERT_EQ(rin.sz, rout.sz);

        ASSERT_EQ(ERR_OK, dsn_file_flush(fout));

        offset += rin.sz;
    }

    ASSERT_EQ((uint64_t)fin_size, offset);
    ASSERT_EQ(ERR_OK, dsn_file_close(fout));
    ASSERT_EQ(ERR_OK, dsn_file_close(fin));

    ASSERT_TRUE(utils::filesystem::file_size("command.copy.txt", fout_size));
    ASSERT_EQ(fin_size, fout_size);
# else
    printf("core.dsn_file test will fail on Mac, to be fixed later (RPC is not affected)\n");
# endif
}

TEST(core, dsn_env)
{
    if(dsn::service_engine::fast_instance().spec().tool == "emulator")
        return;
    uint64_t now1 = dsn_now_ns();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    uint64_t now2 = dsn_now_ns();
    ASSERT_LE(now1 + 1000000, now2);
    uint64_t r = dsn_random64(100, 200);
    ASSERT_LE(100, r);
    ASSERT_GE(200, r);
}

TEST(core, dsn_system)
{
    ASSERT_TRUE(tools::is_engine_ready());
    tools::tool_app* tool = tools::get_current_tool();
    ASSERT_EQ(tool->name(), dsn_config_get_value_string("core", "tool", "", ""));

    int app_count = 5;
    int type_count = 1;
    if (tool->get_service_spec().enable_default_app_mimic)
    {
        app_count++;
        type_count++;
    }   

    {
        dsn_app_info apps[20];
        int count = dsn_get_all_apps(apps, 20);
        ASSERT_EQ(app_count, count);
        std::map<std::string, int> type_to_count;
        for (int i = 0; i < count; ++i)
        {
            type_to_count[apps[i].type] += 1;
        }

        ASSERT_EQ(type_count, static_cast<int>(type_to_count.size()));
        ASSERT_EQ(5, type_to_count["test"]);

        count = dsn_get_all_apps(apps, 3);
        ASSERT_EQ(app_count, count);
    }
}

