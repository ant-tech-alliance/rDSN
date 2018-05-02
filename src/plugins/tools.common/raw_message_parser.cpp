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
 *     xxxx-xx-xx, author, fix bug about xxx
 */
#include "raw_message_parser.h"
#include <dsn/service_api_c.h>
#include <dsn/tool-api/task_spec.h>
#include <dsn/tool-api/network.h>

#ifdef __TITLE__
#undef __TITLE__
#endif
#define __TITLE__ "raw.message.parser"

namespace dsn {

DEFINE_TASK_CODE_RPC(RPC_CALL_RAW_SESSION_DISCONNECT, TASK_PRIORITY_COMMON, THREAD_POOL_DEFAULT)
DEFINE_TASK_CODE_RPC(RPC_CALL_RAW_MESSAGE, TASK_PRIORITY_COMMON, THREAD_POOL_DEFAULT)

//static
void raw_message_parser::notify_rpc_session_disconnected(rpc_session *sp)
{
    if (!sp->is_client())
    {
        auto special_msg = message_ex::create_receive_message_with_standalone_header();
        special_msg->header->context.u.is_request = 1;
        special_msg->local_rpc_code = RPC_CALL_RAW_SESSION_DISCONNECT;
        sp->on_recv_request(special_msg, 0);
    }
}

raw_message_parser::raw_message_parser(bool is_client)
    : message_parser(is_client)
{
    bool hooked = false;
    static std::atomic_bool s_handler_hooked(false);
    if (s_handler_hooked.compare_exchange_strong(hooked, true))
    {
        ddebug("join point on_rpc_session_disconnected registered to notify disconnect with RPC_CALL_RAW_SESSION_DISCONNECT");
        rpc_session::on_rpc_session_disconnected.put_back(
            raw_message_parser::notify_rpc_session_disconnected, 
            "notify disconnect with RPC_CALL_RAW_SESSION_DISCONNECT"
        );
    }
}

void raw_message_parser::reset() {}

message_ex* raw_message_parser::get_message_on_receive(message_reader* reader, /*out*/int& read_next)
{
    if (reader->length() == 0)
    {
        if (reader->data() != nullptr)
            read_next = reader->read_buffer_capacity();
        else
            read_next = reader->block_size();
        return nullptr;
    }
    else
    {
        auto msg_length = reader->length();
        auto new_message = message_ex::create_receive_message_with_standalone_header(reader->range(msg_length));
        auto header = new_message->header;

        header->body_length = msg_length;
        new_message->dheader.rpc_name = "RPC_CALL_RAW_MESSAGE";
        header->context.u.is_request = 1;
        reader->consume(msg_length);
        read_next = 0;

        new_message->local_rpc_code = RPC_CALL_RAW_MESSAGE;
        return new_message;
    }
}

int raw_message_parser::get_buffer_count_on_send(message_ex *msg)
{
    return msg->buffers.size();
}

int raw_message_parser::get_buffers_on_send(message_ex *msg, send_buf *buffers)
{
    //we must skip the message header
    unsigned int offset = sizeof(message_header);
    int i=0;
    for (blob& buf : msg->buffers)
    {
        if (offset >= buf.length())
        {
            offset -= buf.length();
            continue;
        }
        buffers[i].buf = (void*)(buf.data() + offset);
        buffers[i].sz = buf.length() - offset;
        offset = 0;

        if (buffers[i].sz > 0)
            ++i;
    }
    return i;
}

}
