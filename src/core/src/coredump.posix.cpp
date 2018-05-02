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


# ifndef _WIN32

# include "coredump.h"
# include <dsn/tool_api.h>
# include <sys/types.h>
# include <signal.h>
# include <execinfo.h>

# ifdef __TITLE__
# undef __TITLE__
# endif
# define __TITLE__ "coredump"

namespace dsn {
    namespace utils {

        static std::string s_dump_dir;
        static void handle_core_dump(int);
        static void handle_term(int);

        void coredump::init(const char* dump_dir)
        {
            s_dump_dir = dump_dir;

            if (dsn_config_get_value_bool("core", "catch_signal", true, 
                    "whether to catch SEGSEGV and SIGtERM signals")) 
            {
                signal(SIGSEGV, handle_core_dump);
                signal(SIGTERM, handle_term);
            }
        }

        void coredump::write()
        {
            ::dsn::tools::sys_exit.execute(SYS_EXIT_EXCEPTION);
            abort();
        }

        static void handle_core_dump(int signal_id)
        {
            fprintf(stderr, "got signal id: %d\n", signal_id);
            fflush(stderr);

            void *buffer[255];
            const int calls = backtrace(buffer, sizeof(buffer) / sizeof(void *));
            backtrace_symbols_fd(buffer, calls, 1);

            if (dsn_config_get_value_bool("core", "wait_debugger_on_catch_signal", false, 
                    "whether to wait debugger on catching SEGSEGV signals")) 
            {
#if defined(_WIN32)
                fprintf(stderr, "\nPause for debugging on fault (pid = %d)...\n", static_cast<int>(::GetCurrentProcessId()));
#else
                fprintf(stderr, "\nPause for debugging on fault (pid = %d)...\n", static_cast<int>(getpid()));
#endif
                fflush(stderr);
                std::this_thread::sleep_for(std::chrono::seconds(3600000)); 
            }

            /*
             * firstly we must set the sig_handler to default,
             * to prevent the possible inifinite loop
             * for example: an sigsegv in the coredump::write()
             */
            if (signal_id == SIGSEGV)
            {
                signal(SIGSEGV, SIG_DFL);
            }
            coredump::write();
        }

        static void handle_term(int signal_id)
        {
            //printf("got signal id: %d\n", signal_id);
            fflush(stdout);
            dsn_exit(0);
        }
    }
}

# endif

