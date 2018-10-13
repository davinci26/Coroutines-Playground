#include <iostream>
#include <experimental/coroutine>

struct resumable_thing{

    struct promise_type;
    std::experimental::coroutine_handle<promise_type> _coroutine = nullptr;
    
    explicit resumable_thing(std::experimental::coroutine_handle<promise_type> coroutine):
    _coroutine(coroutine){
        std::cout << "Constructed resumable_thing" << std::endl;
    }

    ~resumable_thing(){
        std::cout << "Deleted resumable_thing" << std::endl;
        if (_coroutine)
            _coroutine.destroy();
    }

    resumable_thing() = default;
    resumable_thing(resumable_thing const&) = delete;
    resumable_thing& operator=(resumable_thing const&) = delete;

    resumable_thing(resumable_thing && other):_coroutine(other._coroutine){
        std::cout << "Move Constructed resumable_thing" << std::endl;
        other._coroutine = nullptr;
    }

    resumable_thing& operator=(resumable_thing &&other){
        std::cout << "Move assigned resumable_thing" << std::endl;
        if (this != &other){
            _coroutine = other._coroutine;
            other._coroutine = nullptr;
        }
    }

    void resume(){
        std::cout << "Resuming Cooroutine " << (_coroutine.done() ? " Done" : "Not Done") << std::endl;
        if (!_coroutine.done())
            _coroutine.resume();
    }

    struct promise_type{
        
        promise_type(){
            std::cout << "Constructed promise_type" << std::endl;
        }
        
        ~promise_type() {
            std::cout << "Deleted promise_type" << std::endl;
        }

        auto get_return_object() {
            return resumable_thing({std::experimental::coroutine_handle<promise_type>::from_promise(*this)});
        }

        auto initial_suspend() {
            return std::experimental::suspend_never{};
        }
        
        auto final_suspend() {
            return std::experimental::suspend_always{};
        }

        void return_void(){ }
        
        void unhandled_exception() {
            std::exit(1);
        }
    };

};

resumable_thing counter(){
    std::cout << "Counter: called\n";
    for (unsigned i=1; ; ++i){
        co_await std::experimental::suspend_always{};
        std::cout << "counter resumed #"<< i << " \n";
    }
}

int main() {

    std::cout << "main calling counter:\n";
    resumable_thing the_counter = counter();
    std::cout << "main resuming counter:\n";
    the_counter.resume();
    std::cout << "main resuming counter again:\n";
    the_counter.resume();
    std::cout << "main done\n";
}

// int main() {
//     return await_answer().get();
// }