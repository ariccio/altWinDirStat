// Simple- really

struct SEARCHP
{
	int mode;
	wchar_t* string;
	int len;
	wchar_t* extra;
	int extralen;
	int totallen;
};

SEARCHP* StartSearch(wchar_t* string, int len);
int SearchStr(SEARCHP* pattern, wchar_t* string, int len);
int EndSearch(SEARCHP* pattern);