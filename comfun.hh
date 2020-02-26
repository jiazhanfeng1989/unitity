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
#endif
