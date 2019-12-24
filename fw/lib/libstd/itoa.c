#ifdef CONFIG_USE_LOCAL_STRING_H
#include "_string.h"
#else
#include <string.h>
#endif

//-----------------------------------------------------------------
// reverse: reverse string s in place
//-----------------------------------------------------------------
static void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) 
	{
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
//-----------------------------------------------------------------
// itoa: 
//-----------------------------------------------------------------
char * itoa(int n, char *s, int base)
{
	int i, sign;

	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */

	i = 0;
	do 
	{      
		/* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} 
	while ((n /= 10) > 0);     /* delete it */
	
	if (sign < 0)
		s[i++] = '-';
	
	s[i] = '\0';
	reverse(s);

	return s;
}
