/*
 ==============================================================================
 Name        : Lister.c
 Author      : Michael A. Morris
 Version     : 0.0.1
 Copyright   : Copyright 2017 - GPLv3
 Description : Simple Program in C, ANSI-style
 ==============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	FORM_FEED_CHAR	'\f'

#define	MAX_FILENAME_LENGTH     32
#define	MAX_SOURCE_LINE_LENGTH	256
#define	MAX_PRINT_LINE_LENGTH   80
#define MAX_BACK_TRACK_LENGTH   26
#define MAX_LINES_PER_PAGE      50
#define	DATE_STRING_LENGTH      26

typedef enum {
	FALSE, TRUE,
} BOOLEAN;

// Global Variables

int line_number = 0;
int	page_number = 0;
int	level       = 0;
int	line_count  = MAX_LINES_PER_PAGE;

char source_buffer[MAX_SOURCE_LINE_LENGTH];

char source_name[MAX_FILENAME_LENGTH];
char date[DATE_STRING_LENGTH];

FILE *source_file;

// Function Prototypes

BOOLEAN get_source_line(void);
void init_lister(char name[]);	// void init_lister(char *name);
BOOLEAN get_source_line(void);
void print_line(char line[]);
void print_page_header(void);

//	Main Program

int main(int argc, char *argv[])
{
    init_lister(argv[1]);

    /*
     * Repeatedly call get_source_line to read and print
     * the next source line until the end of file.
     */

    while(get_source_line());

    return 0;
}

/*
 *
 * void init_lister(char name[])    Initialize the Lister globals
 *
 */

void init_lister(char name[])
{
    time_t timer;

    /*
     * Copy the source file name and open the source file.
     */

    strcpy(source_name, name);
    source_file = fopen(source_name, "r");

    /*
     * Set the current date and time in the date string.
     */

    time(&timer);
    strcpy(date, asctime(localtime(&timer)));
}

/*
 * BOOLEAN get_source_line(void)    Read the next line from the source
 *                                  file. If there was one, print it
 *                                  out and return TRUE. Else at the
 *                                  end of file, so return FALSE.
 */

BOOLEAN get_source_line(void)
{
    char print_buffer[MAX_SOURCE_LINE_LENGTH];

    if(NULL != fgets(source_buffer, MAX_SOURCE_LINE_LENGTH, source_file)) {
        ++line_number;
        sprintf(print_buffer, "%4d %d: %s", line_number, level, source_buffer);
        print_line(print_buffer);

        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * void print_line(char line[])	Print out a line. Start a new page if
 *                              current page is full.
 */

void print_line(char line[])
{
    char save_ch[2];
    char *save_chp = NULL;

    if(MAX_LINES_PER_PAGE < ++line_count) {
        print_page_header();
        line_count = 1;
    }

    if(MAX_PRINT_LINE_LENGTH < strlen(line)) {
        save_chp = &line[MAX_PRINT_LINE_LENGTH];

		int i;
		for(i = 1; (' ' != *save_chp) && (MAX_BACK_TRACK_LENGTH > i); i++) {
			--save_chp;
		}
    }

    if(save_chp) {
        save_ch[0] = save_chp[0]; save_chp[0] = '\n';
        save_ch[1] = save_chp[1]; save_chp[1] = '\0';
    }

    printf("%s", line);

    if(save_chp) {
        save_chp[0] = save_ch[0]; save_chp[1] = save_ch[1];

//      int i; for(save_chp -= 8, i = 0; i < 8; i++) save_chp[i] = ' ';

        printf("        "); print_line(save_chp);
    }
}

/*
 * void print_page_header(void)	Print the page header at the top of the
 *                              next page.
 */

void print_page_header(void)
{
    putchar(FORM_FEED_CHAR);
    printf("Page %d   %s   %s\n\n", ++page_number, source_name, date);
}
