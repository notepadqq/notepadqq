#ifndef PROMISE_H
#define PROMISE_H

#include <QTimer>

// Lightweight promise for async use *in the same thread*.

template <typename T>
class Promise;


class ChainableCallback {
public:

};


template <typename T = void>
class Promise {

public:
    Promise() {
        m_private = std::make_shared<Private>();
    }


    // Successfull (void) promise
    /*template <typename U = T,
      typename = typename std::enable_if<std::is_same<U, void>::value>::type>
    Promise() {
      // TODO resolve it
    }*/

    // Successfull promise with value
    /*template <typename U = T,
      typename = typename std::enable_if<!std::is_same<U, void>::value>::type>
    Promise(const U &value) {
      // TODO Resolve it
    }*/

    template <typename U>
    Promise<U> then(std::function<U(T)> fun) { // TODO Shortcut for returning a promise!!
        Promise<U> next;

        auto callback = [next, fun](T input){ next.resolve(fun(input)); };

        m_private->callbacks.push_back(callback);

        return next;
    }

    // FIXME Fix this... it should return a Promise<void> with a resolve(void) method...
    Promise<int> then(std::function<void(T)> fun) {
        std::function<int(T)> f = [fun](T v){ fun(v); return 1; };
        return this->then(f);
    }

    void resolve(T value) const {
        if (m_private->resolved) {
            throw std::runtime_error("This promise is already resolved");
        }

        if (m_private->callbacks.size() > 0) {
            m_private->resolved = true;

            auto callbacks = m_private->callbacks;

            QTimer::singleShot(0, [callbacks, value](){ // At next tick, this object might have been destroyed!
                for (auto cb : callbacks) {
                    cb(value);
                }
            });
        }
    }


    Promise(const Promise &other) = default;
    Promise(Promise &&other) = default;
    Promise &operator=(const Promise &other) = default;

    virtual ~Promise() {}

private:

    struct Private {
        std::vector<std::function<void(T)>> callbacks;
        bool resolved = false;
    };

    std::shared_ptr<Private> m_private = nullptr;

};



#endif // PROMISE_H
