#pragma once
class CHttpClient  
{  
public:  
    CHttpClient(void);  
    ~CHttpClient(void);  
  
public:
	//http接口
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);  
    int Get(const std::string & strUrl, std::string & strResponse);  
  
    //https接口
    int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);  
    int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);  
  
public:  
	//调试接口
    void SetDebug(bool bDebug);  
	void SetCookie(const std::string & szCookie) {m_szCookie = szCookie;}
  
private:  
    bool m_bDebug;
	std::string m_szCookie;
	CURL* curl;
};
