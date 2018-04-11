#ifndef PROMISE_H
#define PROMISE_H

#include <QTimer>
#include <memory>

// Lightweight promise for async use *in the same thread*.

template <typename T>
class Promise;


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
    template <typename U = T,
      typename = typename std::enable_if<!std::is_same<U, void>::value>::type>
    Promise(U value) {
        m_private = std::make_shared<Private>();
        this->setResolved(value);
    }

    template <typename U>
    Promise<U> then(std::function<U(T)> fun) {
        Promise<U> next; // Will be resolved after this promise get resolved

        auto callback = [next, fun](T input){ next.resolve(fun(input)); };

        m_private->callbacks.push_back(callback);

        if (m_private->resolved) {
            invokeCallbacks();
        }

        return next;
    }

    template <typename U>
    Promise<U> then(std::function<Promise<U>(T)> fun) {
        Promise<U> next; // Will be resolved after this promise get resolved

        auto callback = [next, fun](T input){
            fun(input).then([next](U value) {
                next.resolve(value);
            });
        };

        m_private->callbacks.push_back(callback);

        if (m_private->resolved) {
            invokeCallbacks();
        }

        return next;
    }

    // FIXME Fix this... it should return a Promise<void> with a resolve(void) method...
    Promise<int> then(std::function<void(T)> fun) {
        std::function<int(T)> f = [fun](T v){ fun(v); return 1; };
        return this->then(f);
    }

    void resolve(T value) const {
        this->setResolved(value);
        this->invokeCallbacks();
    }


    Promise(const Promise &other) = default;
    Promise(Promise &&other) = default;
    Promise &operator=(const Promise &other) = default;

    virtual ~Promise() {}

private:

    struct Private {
        std::vector<std::function<void(T)>> callbacks;
        bool resolved = false;
        T value;
    };

    void setResolved(T value) const {
        if (m_private->resolved) {
            throw std::runtime_error("This promise is already resolved");
        }

        m_private->value = value;
        m_private->resolved = true;
    }

    void invokeCallbacks() const {
        if (m_private->callbacks.size() > 0) {
            // Get a copy of the stuff we need: when we'll use them we
            // might not have this promise alive anymore.
            auto callbacks = m_private->callbacks;
            auto value = m_private->value;

            m_private->callbacks.clear();

            QTimer::singleShot(0, [=](){ // At next tick, this object may not exist anymore!
                for (auto cb : callbacks) {
                    cb(value);
                }
            });
        }
    }

    std::shared_ptr<Private> m_private = nullptr;

};



#endif // PROMISE_H
