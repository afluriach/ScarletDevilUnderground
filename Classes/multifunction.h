//
//  multifunction.h
//  FlansBasement
//
//  Created by Toni on 11/26/15.
//
//

//https://github.com/klmr/multifunction

#ifndef multifunction_h
#define multifunction_h

namespace util {

namespace detail {

    template <typename R, typename... Args>
    struct multifunction_traits {
        typedef R return_type;
    };

    template <typename R, typename Arg>
    struct multifunction_traits<R, Arg> {
        typedef R return_type;
        typedef Arg argument_type;
    };

    template <typename R, typename Arg1, typename Arg2>
    struct multifunction_traits<R, Arg1, Arg2> {
        typedef R return_type;
        typedef Arg1 first_argument_type;
        typedef Arg2 second_argument_type;
    };

    template <typename R, typename... Args>
    struct call_helper {
        static R call(
            std::vector<std::function<R(Args...)>> const& listeners,
            Args... args
        ) {
            R ret;
            for (auto listener : listeners)
                ret = listener(args...);
            return ret;
        }
    };

    template <typename... Args>
    struct call_helper<void, Args...> {
        static void call(
            std::vector<std::function<void(Args...)>> const& listeners,
            Args... args
        ) {
            for (auto listener : listeners)
                listener(args...);
        }
    };

} // namespace detail

template <typename R, typename... Args>
class multifunction;

template <typename R, typename... Args>
class multifunction<R(Args...)> : public detail::multifunction_traits<R, Args...> {
public:

    // Uniquely identifies a listener in this multifunction.
    // This is an opaque type, meaning it can only be created and accessed
    // from within this class. The outside world can only store them.
    class function_token {
        friend class multifunction;

        function_token(long unsigned id) noexcept : id(static_cast<unsigned>(id)) { }

        // We assume that numeric_limits<unsigned>::max() tokens are enough.
        unsigned id;
    };

    multifunction() = default;
    multifunction(multifunction const&) = default;
    multifunction(multifunction&&) = default;
    multifunction& operator =(multifunction const&) = default;
    multifunction& operator =(multifunction&&) = default;
    ~multifunction() = default;

    template <typename F>
    function_token operator +=(F listener) {
        listeners.push_back(listener);
        orderTags.push_back(0);
        token_lookup.push_back(listeners.size() - 1);
        return function_token{token_lookup.size() - 1};
    }

    //Tag allows the call order to specified. Function will be inserted after any function with the same
    //order tag, but before any with a larger tag.
    template <typename F>
    function_token insertWithOrder(F listener, int orderTag) {
        long insertPos = -1;
        
        auto it1 = listeners.begin();
        auto it2 = orderTags.begin();
        for(; it1 != listeners.end(); ++it1, ++it2){
            if(orderTag < *it2){
                insertPos = it1 - listeners.begin();
                
                listeners.insert(it1, listener);
                orderTags.insert(it2, orderTag);
                
                break;
            }
        }
        
        if(insertPos == -1){
            //Insert at end.
            insertPos = listeners.size();
            listeners.push_back(listener);
            orderTags.push_back(orderTag);
        }
    
        token_lookup.push_back(insertPos);
        return function_token{static_cast<unsigned long>(insertPos)};
    }

    void operator -=(function_token token) {
        auto i = token_lookup[token.id];

        if (i == NIL)
            return;

        // Adjust token_lookup positions which have shifted.
        for (auto& token_index : token_lookup)
            if (token_index > i and token_index != NIL)
                --token_index;

        listeners.erase(listeners.begin() + i);
        token_lookup[token.id] = NIL;
    }

    R operator ()(Args... args) const {
        return detail::call_helper<R, Args...>::call(listeners, args...);
    }
    
private:

    std::vector<std::function<R(Args...)>> listeners;
    std::vector<int> orderTags;
    std::vector<std::size_t> token_lookup;
    static constexpr std::size_t NIL = -1;
};

} // namespace util


#endif /* multifunction_h */
