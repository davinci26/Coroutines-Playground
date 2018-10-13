#include <iostream>
#include <iterator>
#include <experimental/coroutine>


template<class T>
struct generator{
    struct promise_type;

    std::experimental::coroutine_handle<promise_type> _coroutine = nullptr;

    explicit generator(std::experimental::coroutine_handle<promise_type> coroutine):
    _coroutine(coroutine){
        std::cout << "Constructed generator" << std::endl;
    }

    ~generator(){
        std::cout << "Deleted generator" << std::endl;
        if (_coroutine)
            _coroutine.destroy();
    }

    generator() = default;
    generator(generator const&) = delete;
    generator& operator=(generator const&) = delete;

    generator(generator && other):_coroutine(other._coroutine){
        std::cout << "Move Constructed generator" << std::endl;
        other._coroutine = nullptr;
    }

    generator& operator=(generator &&other){
        std::cout << "Move assigned generator" << std::endl;
        if (this != &other){
            _coroutine = other._coroutine;
            other._coroutine = nullptr;
        }
    }

    struct promise_type{
        T value;
        generator<T> get_return_object(){
            return generator<T>({std::experimental::coroutine_handle<promise_type>::from_promise(*this)});
        }
        
        auto initial_suspend() {
            return std::experimental::suspend_always{};
        }
    
        auto final_suspend() {
            return std::experimental::suspend_always{};
        }

        auto yield_value(T const& current){
            value = current;
            return std::experimental::suspend_always{};
        }
        
        void unhandled_exception() {
            std::exit(1);
        }
        
        void return_void(){ }
    };

    struct iterator : std::iterator<std::input_iterator_tag,T> {

        std::experimental::coroutine_handle<promise_type> _coroutine=nullptr;
        explicit iterator(){}
        explicit iterator(std::experimental::coroutine_handle<promise_type> coroutine):
        _coroutine(coroutine){
        }

        iterator& operator++(){
            _coroutine.resume();
            if (_coroutine.done()){
                _coroutine = nullptr;
            }
            return *this;
        }

        bool operator != (const iterator &r) const {
            return r._coroutine!=_coroutine;
        }

        T const& operator*() const {
            return _coroutine.promise().value;
        }
    };

    iterator begin(){
        if (_coroutine){
            _coroutine.resume();
            if (_coroutine.done())
                return end();
        }
        return generator::iterator(_coroutine);
    }
    iterator end(){
        return iterator{};
    }


};


generator<int> integers(int first, int second){
    for (auto it = first; it <=second; ++it){
        co_yield it;
    }
}


int main() {

    for (int x: integers(1,10)){
        std::cout << "#" << x << '\n';
    }

}