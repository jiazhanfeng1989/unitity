#ifndef _GET_PASSWD_HH
#define _GET_PASSWD_HH
#include <termio.h>
#include <iostream>

class CPassword
{
    
public:
    CPassword();
    ~CPassword();
    
    void IputPassword();
    std::string GetPassword();
    
private:
    int sh_getch();
    
private:
    std::string mypsw;
};

CPassword::CPassword()
{
    mypsw="";
}

CPassword::~CPassword()
{
    
}

int CPassword::sh_getch(void)
{
    int cr;
    struct termios nts, ots;
    if (tcgetattr(0, &ots) < 0)
    {
       return EOF;
    }

    nts = ots;
    cfmakeraw(&nts);
    if (tcsetattr(0, TCSANOW, &nts) < 0)
    {
        return EOF;
    }

    cr = getchar();
    if (tcsetattr(0, TCSANOW, &ots) < 0)
    {
        return EOF;
    }

    return cr;
}

void CPassword::IputPassword()
{
   std::cout<<"Enter Passwd:";
   int temp_c = -1;
   
   while(true)
   {
       temp_c=sh_getch();
       if(temp_c==13 || temp_c==3 || temp_c==4 ||temp_c==27)
           break;
       mypsw+=temp_c;
       std::cout<<"*";
   }
   std::cout<<std::endl;
}

std::string CPassword::GetPassword()
{
   return mypsw;
}
#endif


