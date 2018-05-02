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
 *     the task abstraction in zion, as well as the derived various types of
 *     tasks in our system
 *
 * Revision history:
 *     Mar., 2015, @imzhenyu (Zhenyu Guo), first version
 *     xxxx-xx-xx, author, fix bug about xxx
 */

# pragma once

# include <dsn/utility/ports.h>
# include <dsn/utility/extensible_object.h>
# include <dsn/tool-api/task_spec.h>
# include <dsn/tool-api/rpc_message.h>
# include <dsn/cpp/callocator.h>
# include <dsn/cpp/auto_codes.h>
# include <dsn/cpp/optional.h>
# include <dsn/utility/misc.h>

namespace dsn 
{

namespace lock_checker 
{
    extern __thread int zlock_exclusive_count;
    extern __thread int zlock_shared_count;
    extern void check_wait_safety();
    extern void check_dangling_lock();
    extern void check_wait_task(task* waitee);
}

class task_worker;
class task_worker_pool;
class service_node;
class task_engine;
class task_queue;
class rpc_engine;
class disk_engine;
class env_provider;
class nfs_node;
class timer_service;
class task;

struct __tls_dsn__
{
    uint32_t      magic;    
    task          *current_task;   

    task_worker   *worker;
    int           worker_index;
    service_node  *node;
    int           node_id;
    int           pool_id;

    rpc_engine    *rpc;
    disk_engine   *disk;
    env_provider  *env;
    nfs_node      *nfs;
    timer_service *tsvc;

    int           last_worker_queue_size;
    uint64_t      node_pool_thread_ids; // 8,8,16 bits
    uint32_t      last_lower32_task_id; // 32bits

    char          scratch_buffer[4][256]; // for temp to_string() etc., 4 buffers in maximum
    int           scratch_buffer_index;
    char*         scratch_next() { return scratch_buffer[++scratch_buffer_index % 4]; }
};

extern __thread struct __tls_dsn__ tls_dsn;

//----------------- common task -------------------------------------------------------

class task :
    public extensible_object<task, 4>
{
public:
    DSN_API task(
        dsn_task_code_t code, 
        void* context, 
        int hash = 0, 
        service_node* node = nullptr
        );
    DSN_API virtual ~task();
        
    virtual void exec() = 0;

    DSN_API void           exec_internal();
    DSN_API virtual void   enqueue();
    DSN_API const char*    node_name() const;
    void                    set_error_code(error_code err) { _error = err; }
    void                    set_delay(int delay_milliseconds = 0) { _delay_milliseconds = delay_milliseconds; }
    void                    set_retry(bool enqueue_immediately = true);
    void                    set_inline() { _allow_inline = true; }

    uint64_t                id() const { return _task_id; }
    task_state              state() const { return _state; }
    dsn_task_code_t        code() const { return _spec->code; }
    task_spec&              spec() const { return *_spec; }
    int                     hash() const { return _hash; }
    int                     delay_milliseconds() const { return _delay_milliseconds; }
    error_code              error() const { return _error; }
    service_node*           node() const { return _node; }    

    // static helper utilities
    DSN_API static task*            get_current_task();
    DSN_API static uint64_t         get_current_task_id();
    DSN_API static task_worker*     get_current_worker();
    DSN_API static task_worker*     get_current_worker2();
    DSN_API static service_node*    get_current_node();
    DSN_API static service_node*    get_current_node2();
    DSN_API static int              get_current_node_id();
    DSN_API static int              get_current_pool_id();
    DSN_API static int              get_current_worker_index();
    DSN_API static const char*      get_current_node_name();
    DSN_API static service_node*    get_first_node();
    DSN_API static rpc_engine*      get_current_rpc();
    DSN_API static disk_engine*     get_current_disk();
    DSN_API static env_provider*    get_current_env();
    DSN_API static nfs_node*        get_current_nfs();
    DSN_API static timer_service*   get_current_tsvc();
    DSN_API static int              get_current_queue_length();

    DSN_API static void     set_tls_dsn_context(
                                service_node* node,  // cannot be null
                                task_worker* worker // null for io or timer threads if they are not worker threads
                                );
    DSN_API static void     set_tls_dsn(const __tls_dsn__* ctx);
    DSN_API static void     get_tls_dsn(/*out*/ __tls_dsn__* ctx);

protected:
    DSN_API void            enqueue(task_worker_pool* pool);
    void                     set_task_id(uint64_t tid) { _task_id = tid;  }
        
    error_code             _error;
    void                   *_context; // the context for the taskl callbacks

private:
    task(const task&);
    static void            check_tls_dsn();
    static void            on_tls_dsn_not_set();

    mutable task_state     _state;
    uint64_t               _task_id;
    int                    _hash;
    int                    _delay_milliseconds;
    task_spec              *_spec;
    service_node           *_node;
    bool                   _allow_inline;
    
public:
    // used by task queue only
    task*                  next;
};

class task_c : public task, public transient_object
{
public:
    task_c(
        dsn_task_code_t code,
        dsn_task_handler_t cb, 
        void* context, 
        int hash = 0, 
        service_node* node = nullptr
        )
        : task(code, context, hash, node)
    {
        _cb = cb;
    }

    void exec() override
    {
        _cb(_context);
    }

    virtual void enqueue() override
    {
        spec().on_task_enqueue.execute(get_current_task(), this);
        task::enqueue();
    }

private:
    dsn_task_handler_t _cb;
};


//----------------- timer task -------------------------------------------------------

class timer_task : public task
{
public:
    timer_task(
        dsn_task_code_t code, 
        dsn_task_handler_t cb, 
        dsn_task_handler_t deletor, 
        void* context,
        uint32_t interval_milliseconds,
        int hash = 0, 
        service_node* node = nullptr
        );

    ~timer_task()
    {
        _deletor(_context);
    }

    DSN_API void exec() override;

    DSN_API void enqueue() override;

    DSN_API void stop();

private:
    std::atomic<uint32_t> _interval_milliseconds;
    dsn_task_handler_t   _deletor;
    dsn_task_handler_t   _cb;
};

//----------------- rpc task -------------------------------------------------------

struct rpc_handler_info
{
    dsn_task_code_t           code;
    safe_string                service_name;
    safe_string                method_name;
    dsn_rpc_request_handler_t c_handler;
    void*                      parameter;

    explicit rpc_handler_info(dsn_task_code_t code)
        : code(code), c_handler(nullptr), parameter(nullptr)
    {
    }
};

class service_node;
class rpc_request_task : public task, public transient_object
{
public:
    rpc_request_task(
        message_ex* request,
        dsn_rpc_request_handler_t handler,
        void* param,
        service_node* node
        );
    ~rpc_request_task();

    message_ex*  get_request() const { return _request; }

    DSN_API void enqueue() override;

    void  exec() override
    {
        if (0 == _enqueue_ts_ns
            || dsn_now_ns() - _enqueue_ts_ns < 
            static_cast<uint64_t>(_request->header->client.timeout_ms) * 1000000ULL)
        {
            if (!_handler(_request, _param))
            {
                // request is handled asynchronously
                _request = nullptr;
            }
        }
    }

protected:
    message_ex*                _request;
    dsn_rpc_request_handler_t _handler;
    void*                      _param;
    uint64_t                   _enqueue_ts_ns;
};

typedef bool (*dsn_rpc_response_handler_replace_t)(
    dsn_rpc_response_handler_t callback,
    dsn_rpc_error_t err,
    dsn_message_t resp,
    void* context,
    uint64_t replace_context
    );
class rpc_response_task : public task, public transient_object
{
public:
    DSN_API rpc_response_task(
        message_ex* request, 
        dsn_rpc_response_handler_t cb,
        void* context, 
        int hash = 0, 
        service_node* node = nullptr
        );
    DSN_API ~rpc_response_task();

    // return true for normal case, false for fault injection applied
    DSN_API bool     enqueue(dsn_rpc_error_t err, message_ex* reply);
    DSN_API void     enqueue() override; // re-enqueue after above enqueue, e.g., after delay
    uint64_t          trace_id() const { return _trace_id; }
    message_ex*       get_response() const    { return _response; }     
    message_ex*       move_response() { auto resp = _response; _response = nullptr; return resp; }
    DSN_API void     replace_callback(dsn_rpc_response_handler_replace_t callback, uint64_t context); // not thread-safe
    DSN_API bool     reset_callback(); // used only when replace_callback is called before, not thread-safe
    task_worker_pool* caller_pool() const { return _caller_pool; }
    void              set_caller_pool(task_worker_pool* pl) { _caller_pool = pl; }
    
    void  exec() override
    {
        if (_cb)
        {
            if (!_cb(_rpc_error, _response, _context))
            {
                // response is used asynchronously
                _response = nullptr;
            }
        }
    }

private:
    uint64_t                   _trace_id;
    message_ex*                _response;
    task_worker_pool *         _caller_pool;
    dsn_rpc_response_handler_t _cb;
    dsn_rpc_error_t           _rpc_error;

    friend class rpc_engine;    
};

//------------------------- disk AIO task ---------------------------------------------------

enum aio_type
{
    AIO_Invalid,
    AIO_Read,
    AIO_Write
};

class disk_engine;
class disk_aio
{
public:    
    // filled by apps
    dsn_handle_t file;
    void*        buffer;
    uint32_t     buffer_size;    
    uint64_t     file_offset;

    // filled by frameworks
    aio_type     type;
    disk_engine *engine;
    void*        file_object;

    disk_aio() : file(nullptr), buffer(nullptr), buffer_size(0), file_offset(0), type(AIO_Invalid), engine(nullptr), file_object(nullptr)
    {}
    virtual ~disk_aio(){}
};

class aio_task : public task, public transient_object
{
public:
    DSN_API aio_task(
        dsn_task_code_t code,
        dsn_aio_handler_t cb, 
        void* context, 
        int hash = 0,
        service_node* node = nullptr
        );
    DSN_API ~aio_task();

    DSN_API void   enqueue(error_code err, size_t transferred_size);
    size_t          get_transferred_size() const { return _transferred_size; }
    disk_aio*       aio() { return _aio; }

    void copy_to(char* dest)
    {
        if (!_unmerged_write_buffers.empty())
        {
            for (auto &buffer : _unmerged_write_buffers)
            {
                memcpy(dest, buffer.buffer, buffer.size);
                dest += buffer.size;
            }
        }
        else
        {
            memcpy(dest, _aio->buffer, _aio->buffer_size);
        }
    }

    void collapse() {
        if (!_unmerged_write_buffers.empty()) {
            std::shared_ptr<char> buffer(dsn::make_shared_array<char>(_aio->buffer_size));
            _merged_write_buffer_holder.assign(buffer, 0, _aio->buffer_size);
            _aio->buffer = buffer.get();
            copy_to(buffer.get());
        }
    }

    virtual void exec() override // aio completed
    {
        if (nullptr != _cb)
        {
            _cb(_error.get(), _transferred_size, _context);
        }
        else
        {
            _error.end_tracking();
        }
    }

    std::vector<dsn_file_buffer_t> _unmerged_write_buffers;
    blob                            _merged_write_buffer_holder;
protected: 
    disk_aio*         _aio;
    size_t            _transferred_size;
    dsn_aio_handler_t _cb;
};

// ------------------------ inline implementations --------------------
__inline /*static*/ void task::check_tls_dsn()
{
    if (tls_dsn.magic != 0xdeadbeef)
    {
        on_tls_dsn_not_set();
    }
}

__inline /*static*/ task* task::get_current_task()
{
    check_tls_dsn();
    return tls_dsn.current_task;
}

__inline /*static*/ uint64_t task::get_current_task_id()
{
    if (tls_dsn.magic == 0xdeadbeef)
        return tls_dsn.current_task ? tls_dsn.current_task->id() : 0;
    else
        return 0;
}


__inline /*static*/ task_worker* task::get_current_worker()
{
    check_tls_dsn();
    return tls_dsn.worker;
}

__inline /*static*/ task_worker* task::get_current_worker2()
{
    return tls_dsn.magic == 0xdeadbeef ? tls_dsn.worker : nullptr;
}

__inline /*static*/ service_node* task::get_current_node()
{
    check_tls_dsn();
    return tls_dsn.node;
}

__inline /*static*/ int task::get_current_node_id()
{
    return tls_dsn.magic == 0xdeadbeef ? tls_dsn.node_id : 0;
}

__inline /*static*/ int task::get_current_pool_id()
{
    return tls_dsn.magic == 0xdeadbeef ? tls_dsn.pool_id : 0;
}

__inline /*static*/ service_node* task::get_current_node2()
{
    return tls_dsn.magic == 0xdeadbeef ? tls_dsn.node : nullptr;
}

__inline /*static*/ int task::get_current_worker_index()
{
    check_tls_dsn();
    return tls_dsn.worker_index;
}

__inline /*static*/ rpc_engine* task::get_current_rpc()
{
    check_tls_dsn();
    return tls_dsn.rpc;
}

__inline /*static*/ disk_engine* task::get_current_disk()
{
    check_tls_dsn();
    return tls_dsn.disk;
}

__inline /*static*/ env_provider* task::get_current_env()
{
    check_tls_dsn();
    return tls_dsn.env;
}

__inline /*static*/ nfs_node* task::get_current_nfs()
{
    check_tls_dsn();
    return tls_dsn.nfs;
}

__inline /*static*/ timer_service* task::get_current_tsvc()
{
    check_tls_dsn();
    return tls_dsn.tsvc;
}

__inline /*static*/ int task::get_current_queue_length()
{
    check_tls_dsn();
    return tls_dsn.last_worker_queue_size;
}

} // end namespace
