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

# pragma once

# include <dsn/utility/ports.h>
# include <dsn/utility/singleton.h>
# include <dsn/tool-api/global_config.h>
# include <dsn/cpp/auto_codes.h>
# include <sstream>
# include <dsn/utility/synchronize.h>
# include "app_manager.h"

namespace dsn { 

class task_engine;
class rpc_engine;
class disk_engine;
class env_provider;
class logging_provider;
class nfs_node;
class memory_provider;
class task_queue;
class task_worker_pool;
class timer_service;
class aio_provider;

//
//
//
class service_node
{
public:
    struct io_engine
    {        
        rpc_engine*      rpc;
        disk_engine*     disk;
        nfs_node*        nfs;
        timer_service*   tsvc;
        aio_provider     *aio;

        io_engine()
        {
            memset((void*)this, 0, sizeof(io_engine));
        }
    };

public:
    explicit service_node(service_app_spec& app_spec);
       
    rpc_engine*  rpc() const { return _node_io.rpc; }
    disk_engine* disk() const { return _node_io.disk; }
    nfs_node* nfs() const { return _node_io.nfs; }
    timer_service* tsvc() const { return _node_io.tsvc; }
    task_engine* computation() const { return _computation; }

    void get_runtime_info(const safe_string& indent, const safe_vector<safe_string>& args, /*out*/ safe_sstream& ss);
    void get_queue_info(/*out*/ safe_sstream& ss);

    error_code start_io_engine_in_node_start_task();

    ::dsn::error_code start();
    dsn_error_t start_app();

    int id() const { return _app_spec.id; }
    const char* name() const { return _app_spec.pname.c_str(); }
    const service_app_spec& spec() const { return _app_spec;  }
    void* get_app_context_ptr() const { return _app_info.app.app_context_ptr; }

    bool  rpc_register_handler(rpc_handler_info* handler, dsn_gpid gpid);
    rpc_handler_info* rpc_unregister_handler(dsn_task_code_t rpc_code, dsn_gpid gpid, const char* service_name);

    dsn_app_info* get_l1_info() { return &_app_info; }
    app_manager& get_l2_handler() { return _framework; }
    bool handle_l2_rpc_request(dsn_gpid gpid, bool is_write, dsn_message_t req);
    rpc_request_task* generate_l2_rpc_request_task(message_ex* req);

    static dsn_error_t start_app(void* app_context, const safe_string& args, dsn_app_start start, const safe_string& app_name);

private:
    dsn_app_info     _app_info;
    
    service_app_spec _app_spec;
    task_engine*     _computation;

    io_engine                                   _node_io;

    // when this app is hosted by a layer2 handler app
    app_manager                                 _framework;
    rpc_handler_info                            _layer2_rpc_redsn_handler;
    rpc_handler_info                            _layer2_rpc_write_handler;

private:
    error_code init_io_engine();
    error_code start_io_engine_in_main();
};

typedef std::map<int, service_node*> service_nodes_by_app_id;
class service_engine : public utils::singleton<service_engine>
{
public:
    service_engine();

    //ServiceMode Mode() const { return _spec.Mode; }
    const service_spec& spec() const { return _spec; }
    env_provider* env() const { return _env; }
    logging_provider* logging() const { return _logging; }
    memory_provider* memory() const { return _memory; }
    static safe_string get_runtime_info(const safe_vector<safe_string>& args);
    static safe_string get_queue_info(const safe_vector<safe_string>& args);

    void init_before_toollets(const service_spec& spec);
    void init_after_toollets();
    void configuration_changed();

    service_node* start_node(service_app_spec& app_spec);
    void register_system_rpc_handler(dsn_task_code_t code, const char* name, dsn_rpc_request_handler_t cb, void* param, int port = -1); // -1 for all nodes
    const service_nodes_by_app_id& get_all_nodes() const { return _nodes_by_app_id; }

private:
    service_spec                    _spec;
    env_provider*                   _env;
    logging_provider*               _logging;
    memory_provider*                _memory;

    // <port, servicenode>    
    typedef std::map<int, service_node*> node_engines_by_port; // multiple ports may share the same node
    service_nodes_by_app_id         _nodes_by_app_id;
    node_engines_by_port            _nodes_by_app_port;
};

// ------------ inline impl ---------------------

} // end namespace
