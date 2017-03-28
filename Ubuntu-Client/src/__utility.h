utility.h 

inline std::string time2str(const time_t &t) {
#ifdef _WIN32
    char buf[50];
    tm tm_;
    localtime_s(&tm_, &t);
    strftime(buf, 50, "%Y-%m-%d %H:%M:%S", &tm_);
    return std::string(buf);
#elif __linux__
    char buff[50];
    strftime(buff, 50, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buff;
#endif
}

