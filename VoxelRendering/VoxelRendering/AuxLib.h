#pragma once
#include <sys/timeb.h>
class aux {
public:
	static int get_milli_count() {
        timeb tb;
        ftime(&tb);
        int nCount = tb.millitm + (tb.time & 0xfffff) * 1000;
        return nCount;
    }
};

