struct barrier {
    std::atomic<unsigned> count;
    std::atomic<unsigned> spaces;
    std::atomic<unsigned> generation;

    barrier(unsigned count_):
        count(count_), spaces(count_), generation(0) {
    }

    void wait() {
        unsigned const gen = generation.load();
        if (!--spaces) {
            spaces = count.load();
            ++generation;
        } else {
            while (!generation.load() == gen) {
                std::this_thread::yield();
            }
        }
    }

    void done_waiting() {
        --count;
        if (!--spaces) {
            spaces = count.load();
            ++generation;
        }
    }
};
