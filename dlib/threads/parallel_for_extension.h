// Copyright (C) 2013  Davis E. King (davis@dlib.net)
// License: Boost Software License   See LICENSE.txt for the full license.
#ifndef DLIB_PARALLEL_FoR_H__
#define DLIB_PARALLEL_FoR_H__ 

#include "parallel_for_extension_abstract.h"
#include "thread_pool_extension.h"

namespace dlib
{

// ----------------------------------------------------------------------------------------

    namespace impl
    {

        template <typename T>
        class helper_parallel_for
        {
        public:
            helper_parallel_for (
                T& obj_,
                void (T::*funct_)(long)
            ) : 
                obj(obj_),
                funct(funct_)
            {}

            T& obj;
            void (T::*funct)(long);

            void process_block (long begin, long end)
            {
                for (long i = begin; i < end; ++i)
                    (obj.*funct)(i);
            }
        };
        
        template <typename T>
        class helper_parallel_for_funct
        {
        public:
            helper_parallel_for_funct (
                const T& funct_
            ) : funct(funct_) {}

            const T& funct;

            void run(long i)
            {
                funct(i);
            }
        };

        template <typename T>
        class helper_parallel_for_funct2
        {
        public:
            helper_parallel_for_funct2 (
                const T& funct_
            ) : funct(funct_) {}

            const T& funct;

            void run(long begin, long end)
            {
                funct(begin, end);
            }
        };
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for_blocked (
        thread_pool& tp,
        long begin,
        long end,
        T& obj,
        void (T::*funct)(long, long),
        long chunks_per_thread = 4
    )
    {
        if (tp.num_threads_in_pool() != 0)
        {
            const long num = end-begin;
            const long num_workers = static_cast<long>(tp.num_threads_in_pool());
            // How many samples to process in a single task (aim for chunks_per_thread jobs per worker)
            const long block_size = std::max(1L, num/(num_workers*chunks_per_thread));
            for (long i = 0; i < num; i+=block_size)
            {
                tp.add_task(obj, funct, i, std::min(i+block_size, num));
            }
            tp.wait_for_all_tasks();
        }
        else
        {
            // Since there aren't any threads in the pool we might as well just invoke
            // the function directly since that's all the thread_pool object would do.
            // But doing it ourselves skips a mutex lock.
            (obj.*funct)(begin, end);
        }
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for_blocked (
        unsigned long num_threads,
        long begin,
        long end,
        T& obj,
        void (T::*funct)(long, long),
        long chunks_per_thread = 4
    )
    {
        thread_pool tp(num_threads);
        parallel_for_blocked(tp, begin, end, obj, funct, chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for_blocked (
        thread_pool& tp,
        long begin,
        long end,
        const T& funct,
        long chunks_per_thread = 4
    )
    {
        impl::helper_parallel_for_funct2<T> helper(funct);
        parallel_for_blocked(tp, begin, end,  helper, &impl::helper_parallel_for_funct2<T>::run,  chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for_blocked (
        unsigned long num_threads,
        long begin,
        long end,
        const T& funct,
        long chunks_per_thread = 4
    )
    {
        thread_pool tp(num_threads);
        parallel_for_blocked(tp, begin, end, funct, chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for (
        thread_pool& tp,
        long begin,
        long end,
        T& obj,
        void (T::*funct)(long),
        long chunks_per_thread = 4
    )
    {
        impl::helper_parallel_for<T> helper(obj, funct);
        parallel_for_blocked(tp, begin, end, helper, &impl::helper_parallel_for<T>::process_block, chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for (
        unsigned long num_threads,
        long begin,
        long end,
        T& obj,
        void (T::*funct)(long),
        long chunks_per_thread = 4
    )
    {
        thread_pool tp(num_threads);
        parallel_for(tp, begin, end, obj, funct, chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for (
        thread_pool& tp,
        long begin,
        long end,
        const T& funct,
        long chunks_per_thread = 4
    )
    {
        impl::helper_parallel_for_funct<T> helper(funct);
        parallel_for(tp, begin, end,  helper, &impl::helper_parallel_for_funct<T>::run,  chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------

    template <typename T>
    void parallel_for (
        unsigned long num_threads,
        long begin,
        long end,
        const T& funct,
        long chunks_per_thread = 4
    )
    {
        thread_pool tp(num_threads);
        parallel_for(tp, begin, end, funct, chunks_per_thread);
    }

// ----------------------------------------------------------------------------------------

}

#endif // DLIB_PARALLEL_FoR_H__

