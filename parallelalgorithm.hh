#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include <algorithm>

template<typename Iterator,typename Func>
void parallel_for_each(Iterator first,Iterator last,Func f)
{
    unsigned long const length=std::distance(first,last);

    if(!length)
        return;

    unsigned long const min_per_thread=25;
    unsigned long const max_threads=(length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads=std::thread::hardware_concurrency();
    unsigned long const num_threads=std::min(hardware_threads!=0?hardware_threads:2,max_threads);
    unsigned long const block_size=length/num_threads;

    std::vector<std::future<void> > futures(num_threads-1);
    std::vector<std::thread> threads(num_threads-1);
    join_threads joiner(threads);

    Iterator block_start=first;
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        Iterator block_end=block_start;
        std::advance(block_end,block_size);
        std::packaged_task<void(void)> task(
            [=]()
            {
                std::for_each(block_start,block_end,f);
            });
        futures[i]=task.get_future();
        threads[i]=std::thread(std::move(task));
        block_start=block_end;
    }
    std::for_each(block_start,last,f);
    for(unsigned long i=0;i<(num_threads-1);++i)
    {
        futures[i].get();
    }
}

template<typename Iterator,typename Func>
void parallel_for_each_aysnc(Iterator first,Iterator last,Func f)
{
    unsigned long const length=std::distance(first,last);

    if(!length)
        return;

    unsigned long const min_per_thread=25;

    if(length<(2*min_per_thread))
    {
        std::for_each(first,last,f);
    }
    else
    {
        Iterator const mid_point=first+length/2;
        std::future<void> first_half= std::async(&parallel_for_each<Iterator,Func>,first,mid_point,f);
        parallel_for_each(mid_point,last,f);
        first_half.get();
    }
}

template<typename Iterator,typename MatchType>
Iterator parallel_find(Iterator first,Iterator last,MatchType match)
{
    struct find_element
    {
        void operator()(Iterator begin,Iterator end,
                        MatchType match,
                        std::promise<Iterator>* result,
                        std::atomic<bool>* done_flag)
        {
            try
            {
                for(;(begin!=end) && !done_flag->load();++begin)
                {
                    if(*begin==match)
                    {
                        result->set_value(begin);
                        done_flag->store(true);
                        return;
                    }
                }
            }
            catch(...)
            {
                try
                {
                    result->set_exception(std::current_exception());
                    done_flag->store(true);
                }
                catch(...)
                {}
            }
        }
    };

    unsigned long const length=std::distance(first,last);

    if(!length)
        return last;

    unsigned long const min_per_thread=25;
    unsigned long const max_threads=(length+min_per_thread-1)/min_per_thread;
    unsigned long const hardware_threads=std::thread::hardware_concurrency();
    unsigned long const num_threads=std::min(hardware_threads!=0?hardware_threads:2,max_threads);
    unsigned long const block_size=length/num_threads;

    std::promise<Iterator> result;
    std::atomic<bool> done_flag(false);
    std::vector<std::thread> threads(num_threads-1);
    {
        join_threads joiner(threads);

        Iterator block_start=first;
        for(unsigned long i=0;i<(num_threads-1);++i)
        {
            Iterator block_end=block_start;
            std::advance(block_end,block_size);
            threads[i]=std::thread(find_element(),block_start,block_end,match,&result,&done_flag);
            block_start=block_end;
        }
        find_element()(block_start,last,match,&result,&done_flag);
    }
    if(!done_flag.load())
    {
        return last;
    }
    return result.get_future().get();
}

template<typename Iterator,typename MatchType>
Iterator parallel_find_impl(Iterator first,Iterator last,MatchType match,
                            std::atomic<bool>& done)
{
    try
    {
        unsigned long const length=std::distance(first,last);
        unsigned long const min_per_thread=25;
        if(length<(2*min_per_thread))
        {
            for(;(first!=last) && !done.load();++first)
            {
                if(*first==match)
                {
                    done=true;
                    return first;
                }
            }
            return last;
        }
        else
        {
            Iterator const mid_point=first+(length/2);
            std::future<Iterator> async_result=std::async(&parallel_find_impl<Iterator,MatchType>,mid_point,last,match,std::ref(done));
            Iterator const direct_result= parallel_find_impl(first,mid_point,match,done);
            return (direct_result==mid_point)?async_result.get():direct_result;
        }
    }
    catch(...)
    {
        done=true;
        throw;
    }
}

template<typename Iterator,typename MatchType>
Iterator parallel_find_async(Iterator first,Iterator last,MatchType match)
{
    std::atomic<bool> done(false);
    return parallel_find_impl(first,last,match,done);
}



struct join_threads
{
    join_threads(std::vector<std::thread>&)
    {}
};
    

struct barrier
{
    std::atomic<unsigned> count;
    std::atomic<unsigned> spaces;
    std::atomic<unsigned> generation;
    barrier(unsigned count_):count(count_),spaces(count_),generation(0)
    {}
    void wait()
    {
        unsigned const gen=generation.load();
        if(!--spaces)
        {
            spaces=count.load();
            ++generation;
        }
        else
        {
            while(generation.load()==gen)
            {
                std::this_thread::yield();
            }
        }
    }

    void done_waiting()
    {
        --count;
        if(!--spaces)
        {
            spaces=count.load();
            ++generation;
        }
    }
};

template<typename Iterator>
void parallel_partial_sum(Iterator first,Iterator last)
{
    typedef typename Iterator::value_type value_type;

    struct process_element
    {
        void operator()(Iterator first,Iterator last,
                        std::vector<value_type>& buffer,
                        unsigned i,barrier& b)
        {
            value_type& ith_element=*(first+i);
            bool update_source=false;
            for(unsigned step=0,stride=1;stride<=i;++step,stride*=2)
            {
                value_type const& source=(step%2)?buffer[i]:ith_element;
                value_type& dest=(step%2)?ith_element:buffer[i];
                value_type const& addend=(step%2)?buffer[i-stride]:*(first+i-stride);
                dest=source+addend;
                update_source=!(step%2);
                b.wait();
            }
            if(update_source)
            {
                ith_element=buffer[i];
            }
            b.done_waiting();
        }
    };

    unsigned long const length=std::distance(first,last);

    if(length<=1)
        return;

    std::vector<value_type> buffer(length);
    barrier b(length);
    std::vector<std::thread> threads(length-1);
    join_threads joiner(threads);

    Iterator block_start=first;
    for(unsigned long i=0;i<(length-1);++i)
    {
        threads[i]=std::thread(process_element(),first,last,std::ref(buffer),i,std::ref(b));
    }
    process_element()(first,last,buffer,length-1,b);
}

