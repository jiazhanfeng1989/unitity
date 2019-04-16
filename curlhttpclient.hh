#pragma once
class CHttpClient  
{  
public:  
    CHttpClient(void);  
    ~CHttpClient(void);  
  
public:
	//http�ӿ�
    int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);  
    int Get(const std::string & strUrl, std::string & strResponse);  
  
    //https�ӿ�
    int Posts(const std::string & strUrl, const std::string & strPost, std::string & strResponse, const char * pCaPath = NULL);  
    int Gets(const std::string & strUrl, std::string & strResponse, const char * pCaPath = NULL);  
  
public:  
	//���Խӿ�
    void SetDebug(bool bDebug);  
	void SetCookie(const std::string & szCookie) {m_szCookie = szCookie;}
  
private:  
    bool m_bDebug;
	std::string m_szCookie;
	CURL* curl;
};
