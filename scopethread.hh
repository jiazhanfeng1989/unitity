#ifnde _SCOPE_THREAD_HH
#define _SCOPE_THREAD_HH
#include <thread>
class scoped_thread
{
    std::thread t;
public:
    explicit scoped_thread(std::thread t_):
        t(std::move(t_))
    {
        if(!t.joinable())
            throw std::logic_error("No thread");
    }
    ~scoped_thread()
    {
        t.join();
    }
    scoped_thread(scoped_thread const&)=delete;
    scoped_thread& operator=(scoped_thread const&)=delete;
};

class ScopeGuard
{
public:
    template<typename T>
    explicit ScopeGuard(T&& f)
        : f_(std::forward<T>(f))
    {
        assert(f_);
    }
    
    /**
     * @brief Destructor of scope guard, will call injected function.
     */
    virtual ~ScopeGuard()
    {
        if (f_)
        {
            f_();
        }
    }
public:
    /**
     * @brief Cancel function call when exit current scope.
     * @param N/A
     * @return N/A
     */
    void dismiss()
    {
        f_ = {};
    }
private:
    std::function<void ()> f_;
};  //  class ScopeGuard
#endif
