#ifdef DEBUG_MODE

#include <iostream>
#include <fstream>

class Logger {
public:
    template<typename... Args>
    static void log(const Args&... args) {
        std::ofstream ofs(m_debugFileName, std::ios_base::app);
        if (ofs.fail()) {
            std::cerr << "Unable to open debug file!" << std::endl;
            return;
        }
        logHelper(ofs, args...);
        ofs << std::endl;
    }
protected:
    template<typename T1>
    static void logHelper(std::ofstream &ofs, const T1 &t1) {
        ofs << t1;
    }
    template<typename T1, typename... Tn>
    static void logHelper(std::ofstream &ofs, const T1 &t1, const Tn&... args) {
        ofs << t1;
        logHelper(ofs, args...);
    }
    static const char *m_debugFileName;
};

const char *Logger::m_debugFileName = "debugfile.out";

#define log(...) Logger::log(__func__, "(): ", __VA_ARGS__)

#else

#define log(...)

#endif

int main(int argc, char *argv[]) {
#ifdef DEBUG_MODE
    for (int i = 0; i < argc; ++i)
        log(argv[i]);
#endif
    return 0;
}
