/*
 ==============================================================================
 Name        : Tokenizer.c
 Author      : Michael A. Morris
 Version     : 0.0.1
 Copyright   : Copyright 2017 - GPLv3
 Description : Simple Tokenizer in C, ANSI-style
 ==============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	FORM_FEED_CHAR	'\f'
#define EOF_CHAR        '\x7f'

#define	MAX_FILENAME_LENGTH     32
#define	MAX_SOURCE_LINE_LENGTH	256
#define	MAX_PRINT_LINE_LENGTH   80
#define MAX_BACK_TRACK_LENGTH   26
#define MAX_LINES_PER_PAGE      50
#define	DATE_STRING_LENGTH      26
#define MAX_TOKEN_STRING_LENGTH MAX_SOURCE_LINE_LENGTH
#define MAX_CODE_BUFFER_SIZE    4096

#define NUM_BITS_INT			32
#define MAX_POSITIVE_INTEGER    ((1 << (NUM_BITS_INT - 1)) - 1)
#define MAX_NEGATIVE_INTEGER    (1 << NUM_BITS_INT)
#define MAX_INTEGER             MAX_POSITIVE_INTEGER
#define MAX_DIGIT_COUNT         20

typedef enum {
	FALSE, TRUE,
} BOOLEAN;

/*
 * Character Code Types
 */

typedef enum {
	LETTER, DIGIT, SPECIAL, EOF_CODE,
} CHAR_CODE;

/*
 * Token Types
 */

typedef enum {
	NO_TOKEN,
	WORD, NUMBER, PERIOD,
	END_OF_FILE,
	ERROR,
} TOKEN_CODE;

/*
 * Token Name Strings
 */

char *symbol_strings[] = {
	"<no token>",
	"<WORD>", "<NUMBER>", "<PERIOD>",
	"<END OF FILE>",
	"<ERROR>",
};

/*
 * Literal Structure Definition
 */

typedef enum {
	INTEGER_LIT, STRING_LIT,
} LITERAL_TYPE;

typedef struct {
	LITERAL_TYPE type;
	union {
		int  integer;
		char string[MAX_SOURCE_LINE_LENGTH];
	} value;
} LITERAL;

/*
 * Global Variables
 */

char ch;                // Holds the next character read from input

TOKEN_CODE token;       // Indicates the type of token read from input
LITERAL    literal;		// Working store for literals read from input

char source_buffer[MAX_SOURCE_LINE_LENGTH]; // Source file buffer
char token_string[MAX_TOKEN_STRING_LENGTH]; // Token string
char *bufferp      = source_buffer;         // Source Buffer Pointer
int  buffer_offset = 0;                     // Offset into source buffer
char *tokenp       = token_string;          // Token String Pointer

int     digit_count;    // Number of digits in current number
BOOLEAN count_error;    // TRUE if digit_count > MAX_DIGIT_COUNT

int  line_number = 0;                   // Current line number
int	 line_count  = MAX_LINES_PER_PAGE;  // Number of lines on current page
int	 page_number = 0;                   // Current page number
int	 level       = 0;                   // Current lexical level

char source_name[MAX_FILENAME_LENGTH];  // Buffer for file name argument
char date[DATE_STRING_LENGTH];          // Buffer for calculated date string

FILE *source_file;      // Structure for input data stream

CHAR_CODE char_table[256];  // Character type code associated char codes

/*
 * Macro for converting ch into character type code
 */

#define char_code(ch) char_table[(int) ch]    // no semicolon here, supplied by pgm

// Function Prototypes

void init_scanner(char name[]);
void quit_scanner(void);

void get_token(void);
void get_word(void);
void get_number(void);
void get_special(void);
void print_token(void);

void open_source_file(char name[]);
void close_source_file(void);
void get_char(void);
void skip_blanks(void);
BOOLEAN get_source_line(void);
void print_line(char line[]);
void init_page_header(char name[]);
void print_page_header(void);

/*
 * int main(int, char **)   Loop to tokenize source file
 */

int main(int argc, char *argv[])
{
    init_scanner(argv[1]);

    /*
     * Repeatedly get tokens until period or end of file.
     */

    do {
    	get_token();
    	if(END_OF_FILE == token) {
    		printf("\n******* ERROR ******* Unexpected End of File\n");
    		break;
    	}

    	print_token();
    } while(PERIOD != token);

    quit_scanner();

    return 0;
}

/*
 * void init_scanner(char name[])       Initializer scanner/tokenizer globals
 */

void init_scanner(char name[])
{
	int ch;

	/*
	 * Initialize character type table/array
	 */

	for(ch = 0;   ch <  256; ch++) char_table[ch] = SPECIAL;
	for(ch = '0'; ch <= '9'; ch++) char_table[ch] = DIGIT;
	for(ch = 'a'; ch <= 'z'; ch++) char_table[ch] = LETTER;
	for(ch = 'A'; ch <= 'Z'; ch++) char_table[ch] = LETTER;

	char_table[EOF_CHAR] = EOF_CODE;

	init_page_header(name);
	open_source_file(name);
}

/*
 * void quit_scanner(void)  Quit the scanner gracefully
 */

void quit_scanner(void)
{
	close_source_file();
}

/*
 * Token Processing Routines
 *
 *      After extracting a token, ch is the first character after the token.
 */

/*
 * void get_token(void)     Extract a token from the source buffer
 */

void get_token(void)
{
	skip_blanks();
	tokenp = token_string;

	switch(char_code(ch)) {
	case LETTER :
		get_word();             // Words start with letter
	    break;
	case DIGIT :
		get_number();           // Numbers start with digit
		break;
	case EOF_CODE :
		token = END_OF_FILE;
		break;
	default :
		get_special();          // All other characters are SPECIAL
		break;
	}
}

/*
 * void get_word(void)      Extract a word token from the source buffer
 */

void get_word(void)
{
	/*
	 * Extract word: Starts with LETTER, but consists of LETTERs or DIGITs thereafter
	 */

	while((LETTER == char_code(ch)) || (DIGIT == char_code(ch))) {
		*tokenp++ = ch;     // Copy current character to token string
		get_char();         // Get next character
	}

	*tokenp ='\0';          // Mark the end of the token string
	token = WORD;           // Set token type
}

/*
 * void get_number(void)    Extract a number for the source buffer
 */

void get_number(void)
{
	int nvalue      = 0;    // Value of number
	int digit_count = 0;    // Total number of digits in number

	BOOLEAN count_error = FALSE;    // Set to TRUE if digit_count > MAX_DIGIT_COUNT

	do {
		*tokenp++ = ch;

		if(MAX_DIGIT_COUNT > ++digit_count) {
			nvalue = 10 * nvalue + (ch - '0');
		} else {
			count_error = TRUE;
		}

		get_char();         // Get next character from the source buffer
	} while(DIGIT == char_code(ch));

	if(count_error) {
		token = ERROR;
		return;
	}

	literal.type          = INTEGER_LIT;
	literal.value.integer = nvalue;

	*tokenp ='\0';
	token   = NUMBER;
}

/*
 * void get_special(void)   Extract a special token from source file.
 *                          Only PERIOD is defined as special token
 *                          at this time.
 */

void get_special(void)
{
	*tokenp++ = ch;         // Extract special token from source file

	*tokenp = '\0';
	token   = (('.' == ch) ? PERIOD : ERROR);   // Classify special token

	get_char();             // Get next character from source file
}

/*
 * void print_token(void)   Prints a token extracted from the source file
 */

void print_token(void)
{
	char line[MAX_PRINT_LINE_LENGTH];
	char *symbol_string = symbol_strings[token];    // String matching token

	switch(token){
	case NUMBER :
		sprintf(line, "    >> %-16s %d\n",
				symbol_string,
				literal.value.integer     );
		break;

	default :
		sprintf(line, "     >> %-16s %-s\n",
				symbol_string,
				token_string                );
	}

	print_line(line);
}

/*
 * Source file operations
 */

/*
 * void open_source_file(char name[])   Open the input source file,
 *                                      and fetch first character.
 */

void open_source_file(char name[])
{
	if((NULL == name) || (NULL == (source_file = fopen(name, "r")))) {
		printf("\n***** ERROR ***** Failed to open source file\n");
		exit(-1);
	}

	/*
	 * Fetch first character from the source file.
	 * One character look-ahead is required by the scanner and parsing
	 * routines.
	 */

	bufferp = "";	// Initialize the buffer pointer
	get_char();     // Read first line and initialize look-ahead char variable
}


/*
 * void close_source_file(void)     Close the source file
 */

void close_source_file(void)
{
	fclose(source_file);
}

/*
 * void get_char(void)      Sets ch to the next character from the
 *                          source buffer.
 */

void get_char(void)
{
	/*
	 * If at end of current source line, read another line.
	 * If at the end of file, set ch to EOF character and return.
	 */

	if('\0' == *bufferp) {
		if(!get_source_line()) {
			ch = EOF_CHAR;
			return;
		}

		bufferp       = source_buffer;
		buffer_offset = 0;
	}

	ch = *bufferp++;
	if(   ('\n' == ch)      // Convert new-line,
	   || ('\r' == ch)      // carriage return,
	   || ('\t' == ch)      // tab,
	   || ('\f' == ch)      // form feed,
	   || ('\v' == ch)) {   // vertical tab
		ch = ' ';           // into space
	}
}

/*
 * void skip_blanks(void)   Skip over blanks (and other white space)
 *                          and leave ch pointing to first non-blank
 *                          character.
 */

void skip_blanks(void)
{
	while(' ' == ch) get_char();
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
        printf("        ");
        print_line(save_chp);
    }
}

/*
 *
 * void init_page_header(char name[])   Initialize the page header globals
 *
 */

void init_page_header(char name[])
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
 * void print_page_header(void)	Print the page header at the top of the
 *                              next page.
 */

void print_page_header(void)
{
    putchar(FORM_FEED_CHAR);
    printf("Page %d   %s   %s\n\n", ++page_number, source_name, date);
}
