#include "singleton.h"

using namespace std;

Singleton *Singleton::pInstance = nullptr;
mutex Singleton::sMutex;

Singleton &Singleton::instance() {
    static Cleanup cleanup;
    lock_guard<mutex> guard(sMutex);
    if (pInstance == nullptr)
        pInstance = new Singleton();
    return *pInstance;
}

Singleton::Cleanup::~Cleanup() {
    lock_guard<mutex> guard(Singleton::sMutex);
    delete Singleton::pInstance;
    Singleton::pInstance = nullptr;
}
