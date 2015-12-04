#include <mutex>

class Singleton {
public:
    static Singleton &instance();
protected:
    static Singleton *pInstance;
    friend class Cleanup;
    class Cleanup {
    public:
        ~Cleanup();
    };
private:
    Singleton();
    virtual ~Singleton();
    Singleton(const Singleton &);
    Singleton &operator=(const Singleton &);
    static std::mutex sMutex;
};
