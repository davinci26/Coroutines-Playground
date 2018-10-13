## Introduction to Coroutines for C++

### Aknowledgments
Before we start it should be mentioned that this repo is merely a combination of the following resources:
- [CppCon 2016: James McNellis “Introduction to C++ Coroutines"](https://www.youtube.com/watch?v=ZTqHjjm86Bw&)
- [How C++ coroutines work](https://kirit.com/How%20C%2B%2B%20coroutines%20work)
- Notes I took while reading the resources above

*Disclaimer*: All the good parts in this repository should be accredited to the James McNellis and Kirit Sælensminde (authors of the above) and all of the mistakes/bugs are due to insfuccient understanding of coroutines from my side.

### Building the Code

I am writing the code in UNIX like system using Clang compiler (Apple LLVM version 9.1.0 (clang-902.0.39.2)). Hence I am using the `#include <experimental/coroutine>` header and the `std::experimental` namespace to use coroutines. 
To compile a program using coroutines using Clang you will need to pass the following compilation flags `-std=c++17 -fcoroutines-ts`. The command should like something like this:
```
clang++ cooroutine_hello.cpp -std=c++17 -fcoroutines-ts -o coroutine_hello.out
```
## Coroutines Concepts
### Keyword: co_await

The expression `auto result = co_await expression` is unfolded by the compiler as follows:
``` C++
auto&& __a = expression;
if (!__a.await_ready()){ // ask the expression if its already calculated
    __a.await_suspend(handle); //resume the execution
    // Execution breakpoint, control can go back to the caller
}
auto result = __a.await_resume(); // fetch the result
```
As a result to use the `co_await` we must have an expression that supports:
- `bool await_ready()`: Decides if the control is returned to the caller after an await statement
- `void await_suspend(handle)`
-  `T await_resume()`

Typical expressions that support the above are: `std::suspend_always`,`std::suspend_never` and `std::future<T>` (the latter only applies to MSVC compiler)
### Promise type
The struct `promise_type` should implement
-  `void unhandled_exception()`: What to do in case of an exception
-  `void return_void()` or `void return_value(T value_)`: The value can be retrieved if we use have a member variable in our promise and then do `coroutine.promise().value`
-  `auto get_return_object()` returns the parent object
-  `initial_suspend()`: should I suspend after I create the function
-  `final_suspend()`: should I suspend after the function is executed
-  `yield_value(T const& current)`: If we want the promise to support the `co_yield` keyword

The caller and the coroutine communicate using a `promise` object
For the following example we will see how the compiler will unfold the function using the functions above:
1.Coroutine
``` C++
resumable_thing counter(){
    std::cout << "Counter: called\n";
    for (unsigned i=1; ; ++i){
        co_await std::experimental::suspend_always{}; // Control is returned to the caller
        std::cout << "counter resumed #"<< i << " \n";
    }
}
```
2.The compiler will generate a struct:
```C++
struct counter_context{
    resumable_thing::promise_type _promise;
    unisgned i;
    void* _instruction_pointer // make the function resumable
}
```
3.The coroutine will be transformed

``` C++
resumable_thing counter(){
    /********* Compiler injected code   ********* 
    counter_context* _context = new counter_context();
    _return = _context->_promise().get_return_object();
    co_await _context->_promise().initial_suspend();
    *********************************************/
    std::cout << "Counter: called\n";
    for (unsigned i=1; ; ++i){
        co_await std::experimental::suspend_always{};
        /********* Compiler injected code   ********* 
        Move this resumable_thing instance
        Delete this resumable_thing instance
        *********************************************/
        std::cout << "counter resumed #"<< i << " \n";
    }
    /********* Compiler injected code   ********* 
    co_await _context->_promise().final_suspend();
    delete _context;
    *********************************************/
}
```
### Keyword: co_return & co_yield

`co_return` and `co_yield` keywords follow a similar logic with the above with only small changes.
- In a function `co_return value` is replaced by the compiler with `_context->_promise().return_value(value).`
- In a function `co_yield value` is replaced by the compiler with `co_await _context->_promise().yield_value(value).`

## Other References
[Coroutine Standarization Proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/n4680.pdf)
[Resumamble Functions](https://isocpp.org/files/papers/N4402.pdf)
[Yizhang82's blog](http://yizhang82.me/cpp-coroutines-basic-concepts)