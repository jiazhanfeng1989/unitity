#ifndef _COM_FUN_HH
#define _COM_FUN_HH
std::string& trim(std::string& s) 
{
    if (s.empty())          
	    return s;  
   
    for (auto c = s.begin(); c != s.end() && iswspace(*c++););
	    s.erase(s.begin(), --c); 

	for (auto c = s.end(); c != s.begin() && iswspace(*--c););
	    s.erase(++c, s.end());

	return s;
}

// trim from both ends
static std::string& trim(std::string& s)
{
    // trim from start
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) { return !std::isspace(c); }));

    // trim from end
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); }).base(), s.end());

    return s;
}

std::string time_in_HH_MM_SS_MMM()
{
    using namespace std::chrono;

    // get current time
    auto now = system_clock::now();

    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%H:%M:%S"); // HH:MM:SS
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}
#endif
