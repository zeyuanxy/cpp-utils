#ifndef MESSAGING_H
#define MESSAGING_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

namespace messaging {

struct message_base {
    virtual ~message_base() {
    }
};

template <typename Msg>
struct wrapped_message : message_base {
    Msg contents;
    
    explicit wrapped_message(Msg const &contents_) : 
        contents(contents_) {
    }
};

class queue {
    std::mutex m;
    std::condition_variable c;
    std::queue<std::shared_ptr<message_base>> q;
public:
    template <typename T>
    void push(T const &msg) {
        std::lock_guard<std::mutex> lk(m);
        q.push(std::make_shared<wrapped_message<T>>(msg));
        c.notify_all();
    }
    
    std::shared_ptr<message_base> wait_and_pop() {
        std::unique_lock<std::mutex> lk(m);
        c.wait(lk, [&] {return !q.empty();});
        auto res = q.front();
        q.pop();
        return res;
    }
};

class sender {
    queue *q;
public:
    sender() : 
        q(nullptr) {
    }
    
    explicit sender(queue *q_) :
        q(q_) {
    }
    
    template <typename Message>
    void send(Message const &msg) {
        if (q) {
            q->push(msg);
        }
    }
};

template <typename PreviousDispatcher, typename Msg, typename Func>
class TemplateDispatcher {
    queue *q;
    PreviousDispatcher *prev;
    Func f;
    bool chained;

    TemplateDispatcher(TemplateDispatcher const &) = delete;
    TemplateDispatcher& operator=(TemplateDispatcher const &) = delete;

    template <typename Dispatcher, typename OtherMsg, typename OtherFunc>
    friend class TemplateDispatcher;

    void wait_and_dispatch() {
        for (;;) {
            auto msg = q->wait_and_pop();
            if (dispatch(msg))
                break;
        }
    }

    bool dispatch(std::shared_ptr<message_base> const &msg) {
        if (wrapped_message<Msg>* wrapper = dynamic_cast<wrapped_message<Msg>*>(msg.get())) {
            f(wrapper->contents);
            return true;
        } else {
            return prev->dispatch(msg);
        }
    }
public:
    TemplateDispatcher(TemplateDispatcher &&other) : 
        q(other.q), prev(other.prev), f(std::move(other.f)), chained(other.chained) {
        other.chained = true;    
    }
    TemplateDispatcher(queue *q_, PreviousDispatcher *prev_, Func &&f_) : 
        q(q_), prev(prev_), f(std::forward<Func>(f_)), chained(false) {
        prev_->chained = true;
    }

    template <typename OtherMsg, typename OtherFunc>
    TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>
    handle(OtherFunc &&of) {
        return TemplateDispatcher<TemplateDispatcher, OtherMsg, OtherFunc>(q, this, std::forward<OtherFunc>(of));
    }

    ~TemplateDispatcher() noexcept(false) {
        if (!chained) {
            wait_and_dispatch();
        }
    }
};

class close_queue {
};

class dispatcher {
    queue *q;
    bool chained;

    dispatcher(dispatcher const &) = delete;
    dispatcher& operator=(dispatcher const &) = delete;

    template <typename Dispatcher, typename Msg, typename Func>
    friend class TemplateDispatcher;

    void wait_and_dispatch() {
        for (;;) {
            auto msg = q->wait_and_pop();
            dispatch(msg);
        }
    }

    bool dispatch(std::shared_ptr<message_base> const &msg) {
        if (dynamic_cast<wrapped_message<close_queue>*>(msg.get())) {
            throw close_queue();
        }
        return false;
    }
public:
    dispatcher(dispatcher &&other) : 
        q(other.q), chained(other.chained) {
        other.chained = true;
    }

    explicit dispatcher(queue *q_) : 
        q(q_), chained(false) {
    }

    template <typename Message, typename Func>
    TemplateDispatcher<dispatcher, Message, Func>
    handle(Func &&f) {
        return TemplateDispatcher<dispatcher, Message, Func>(q, this, std::forward<Func>(f));
    }

    ~dispatcher() noexcept(false) {
        if (!chained) {
            wait_and_dispatch();
        }
   }
};

class receiver {
    queue q;
public:
    operator sender() {
        return sender(&q);
    }
    dispatcher wait() {
        return dispatcher(&q);
    }
};

};

#endif
