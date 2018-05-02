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

# include <dsn/tool_api.h>
# include <dsn/utility/synchronize.h>
# include "io_looper.h"

# ifdef __linux__
# include <libaio.h>
# endif

# if defined(__APPLE__) || defined(__FreeBSD__)
# include <aio.h>
# include <fcntl.h>
# endif

namespace dsn {
    namespace tools {
        class hpc_aio_provider : public aio_provider
        {
        public:
            hpc_aio_provider(disk_engine* disk, aio_provider* inner_provider);
            virtual ~hpc_aio_provider();

            virtual dsn_handle_t open(const char* file_name, int flag, int pmode) override;
            virtual error_code   close(dsn_handle_t fh) override;
            virtual error_code   flush(dsn_handle_t fh) override;
            virtual void         aio(aio_task* aio) override;
            virtual disk_aio*    prepare_aio_context(aio_task* tsk) override;

            virtual void start() override;

        protected:
            error_code aio_internal(aio_task* aio, bool async, /*out*/ uint32_t* pbytes = nullptr);
            void completion_handler(event_loop*, int fd, int events);
            friend void __hpc_aio_handler(event_loop* lp, int fd, void* ctx, int events);

        private:            
            io_looper *_looper;

# ifdef __linux__
            void complete_aio(struct iocb* io, int bytes, int err);

            io_context_t _ctx;
            int          _event_fd;
# elif defined(__FreeBSD__)
            void complete_aio(struct aiocb* io, int bytes, int err);
# elif defined(__APPLE__)
            friend void hpc_aio_completed(sigval sigval);
# endif
        };
    }
}
