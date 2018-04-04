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
        Callback<U> c;
        c.callback = fun;

        Promise<U> next;
        c.then = next;

        m_private->callbacks.push_back(c);

        return next;
    }

    void resolve(T value) {
        QTimer::singleShot(0, [this, value](){
            for (auto cb : m_private->callbacks) {
                auto result = cb.callback(value);
                for (auto then : cb.then) {
                    then.resolve(result);
                }
            }
        });
    }


    // TODO: explicitly remove context?
    //Promise(const Promise &other) = default;
    //Promise(Promise &&other) = default;
    //Promise &operator=(const Promise &other) = default;

    virtual ~Promise() {}

private:

    struct Private {
        std::vector<ChainableCallback> callbacks;
    };

    template <typename U>
    struct Callback: ChainableCallback {
        std::function<U(T)> callback;
        std::vector<Promise<U>> then; // promises chained with .then
    };

    std::shared_ptr<Private> m_private = std::make_shared<Private>();

};



#endif // PROMISE_H
