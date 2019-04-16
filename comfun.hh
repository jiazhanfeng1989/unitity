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
#endif