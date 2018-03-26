#include	"mk.h"

/*
 *	This file contains functions that depend on nash's syntax. 
 */

/*
 *	skip a token in quotes.
 */
static char *
nashsquote(char *cp, int c)
{
	Rune r;
	int n;

	while(*cp){
		n = chartorune(&r, cp);
		if(r == c)
			return cp;
		if(r == '\\')
			n += chartorune(&r, cp+n);
		cp += n;
	}
	SYNERR(-1);		/* should never occur */
	fprint(2, "missing closing '\n");
	return 0;
}
/*
 *	search a string for unescaped characters in a pattern set
 */
static char *
nashcharin(char *cp, char *pat)
{
	Rune r;
	int n, vargen;

	vargen = 0;
	while(*cp){
		n = chartorune(&r, cp);
		switch(r){
		case '\\':			/* skip escaped char */
			cp += n;
			n = chartorune(&r, cp);
			break;
		case '"':
			cp = nashsquote(cp+1, r);	/* n must = 1 */
			if(!cp)
				return 0;
			break;
		case '$':
			if(*(cp+1) == '{')
				vargen = 1;
			break;
		case '}':
			if(vargen)
				vargen = 0;
			else if(utfrune(pat, r))
				return cp;
			break;
		default:
			if(vargen == 0 && utfrune(pat, r))
				return cp;
			break;
		}
		cp += n;
	}
	if(vargen){
		SYNERR(-1);
		fprint(2, "missing closing } in pattern generator\n");
	}
	return 0;
}

/*
 *	extract an escaped token.  Possible escape chars are single-quote,
 *	double-quote,and backslash.  Nash only has double-quotes and
 * 	backslack have to be inserted again into buffer.
 */
char*
nashexpandquote(char *s, Rune r, Bufblock *b)
{
	if(r != '"') {
		rinsert(b, r);
		return s;
	}

	/* inside double-quote */
	while(*s){
		s += chartorune(&r, s);
		if(r == '"') {
			if(*s == '"') 
				s++;
			else
				return s;
		}
		rinsert(b, r); /* consumes the string */
	}
	
	return 0;
}

/*
 *	Input an escaped token.  Possible escape chars are single-quote,
 *	double-quote and backslash.  Only the first is a valid escape for
 *	rc; the others are just inserted into the receiving buffer.
 */
int
nashescapetoken(Biobuf *bp, Bufblock *buf, int preserve, int esc)
{
	SYNERR(-1); 
	return 0;
}

/*
 *	check for quoted strings.
 *	s points to char after opening quote, q.
 */
char *
nashcopyq(char *s, Rune q, Bufblock *buf)
{
	Rune r;
	if(q == '"') {
		while(*s){
			s += chartorune(&r, s);
			rinsert(buf, r);
			if(r == '"')
				break;
		}
	}
			
	return s;
}

static int
nashmatchname(char *name)
{
	char *p;

	if((p = strrchr(name, '/')) != nil)
		name = p+1;
	if(strlen(name)!=4)
		return 0;

	if(strcmp(name, "nash") == 0)
		return 1;
	return 0;
}

Shell nashshell = {
	"nash",
	"\"= \t",

	/* 	nash doesn't have IFS but otherwise we
		cannot access lists from mk */
	' ', 
	nashcharin,
	nashexpandquote,
	nashescapetoken,
	nashcopyq,
	nashmatchname
};
