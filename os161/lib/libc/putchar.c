#include <stdio.h>
#include <unistd.h>

/*
 * C standard function - print a single character.
 *
 * Properly, stdio is supposed to be buffered, but for present purposes
 * writing that code is not really worthwhile.
 */

int
putchar(int ch)
{
	//char c = ch;
	//int len;
	printchar(ch);
	/*if (len<=0) {
		return EOF;
	}*/
	return ch;
}
