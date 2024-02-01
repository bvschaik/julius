#ifndef _SXML_H_INCLUDED
#define _SXML_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*
 --- SXML ---
 Short description of how to use SXML for parsing XML text.

 SXML is a lightweight XML parser with no external dependencies.
 To parse XML text you only need to call one function: sxml_parse().
 The function has the following return codes:
*/

typedef enum
{
	SXML_ERROR_XMLINVALID= -1,	/* Parser found invalid XML data - not much you can do beyond error reporting */
	SXML_SUCCESS= 0,			/* Parser has completed successfully - parsing of XML document is complete */
	SXML_ERROR_BUFFERDRY= 1,	/* Parser ran out of input data - refill buffer with more XML text to continue parsing */
	SXML_ERROR_TOKENSFULL= 2	/* Parser has filled all the supplied tokens with data - provide more tokens for further output */
} sxmlerr_t;

/*
 You provide sxml_parse() with a buffer of XML text for parsing.
 The parser will handle text data encoded in ascii, latin-1 and utf-8.
 It should also work with other encodings that are acsii extensions.

 sxml_parse() is reentrant.
 In the case of return code SXML_ERROR_BUFFERDRY or SXML_ERROR_TOKENSFULL, you are expected to call the function again after resolving the problem to continue parsing.
 */

typedef	struct sxml_t sxml_t;
typedef	struct sxmltok_t sxmltok_t;
sxmlerr_t sxml_parse(sxml_t *parser, const char *buffer, unsigned bufferlen, sxmltok_t* tokens, unsigned num_tokens);

/*
 The sxml_t object stores all data required for SXML to continue from where it left of.

 After calling sxml_parse() 'ntokens' tells you how many output tokens have been filled with data.
 Depending on how you resolve SXML_ERROR_BUFFERDRY or SXML_ERROR_TOKENSFULL you may need to modify 'bufferpos' and 'ntokens' to correctly reflect the new buffer and tokens you provide.
*/

struct sxml_t
{
	unsigned bufferpos;	/* Current offset into buffer - all XML data before this position has been successfully parsed */
	unsigned ntokens;	/* Number of tokens filled with valid data by the parser */
	unsigned taglevel;	/* Used internally - keeps track of number of unclosed XML elements to detect start and end of document */
};

/*
 Before you call sxml_parse() for the first time, you have to initialize the parser object.
 You may easily do that with the provided function sxml_init().
*/

void sxml_init(sxml_t *parser);

/*
 Unlike most XML parsers, SXML does not use SAX callbacks or allocate a DOM tree.
 Instead you will have to interpret the XML structure through a table of tokens.

 A token can describe any of the following types:
*/

typedef enum
{
	SXML_STARTTAG,	/* Start tag describes the opening of an XML element */
	SXML_ENDTAG,	/* End tag is the closing of an XML element */

	SXML_CHARACTER,		/* Character data may be escaped - check if the first character is an ampersand '&' to identity a XML character reference */
	SXML_CDATA,			/* Character data should be read as is - it is not escaped */

	/* And some other token types you might be interested in: */
	SXML_INSTRUCTION,	/* Can be used to identity the text encoding */
	SXML_DOCTYPE,		/* If you'd like to interpret DTD data */
	SXML_COMMENT		/* Most likely you don't care about comments - but this is where you'll find them */
} sxmltype_t;

/*
 If you are familiar with the structure of an XML document most of these type names should sound familiar.
 
 A token has the following data:
*/

struct sxmltok_t
{
	unsigned short type;	/* A token is one of the above sxmltype_t */
	unsigned short size;	/* The following number of tokens contain additional data related to this token - used for describing attributes */

	/* 'startpos' and 'endpos' together define a range within the provided text buffer - use these offsets with the buffer to extract the text value of the token */
	unsigned startpos;
	unsigned endpos;
};

/*
 Let's walk through how to correctly interpret a token of type SXML_STARTTAG.
 
 <example zero='' one='Hello there!' three='Me, Myself &amp; I' />

 The element name ('example') can be extracted from the text buffer using 'startpos' and 'endpos'.

 The attributes of the XML element are described in the following 'size' tokens.
 Each attribute is divided by a token of type SXML_CDATA - this is the attribute key.
 There will be zero or more tokens of type SXML_CHARACTER following the key - together they describe one attribute value.
 
 In our example you will get the following number of SXML_CHARACTER tokens after the attribute key:
 * 'zero' will use no tokens to describe the empty attribute value.
 * 'one' will have one token describing the attribute value ('Hello there!').
 * 'three' will have three tokens describing the attribute value ('Me, Myself ')('&amp;')(' I')

 In our example the token of type SXML_STARTTAG will have a 'size' of 7 (3 SXML_CDATA and 4 SXML_CHARACTER).
 When processing the tokens do not forget about 'size' - for any token you want to skip, also remember to skip the additional token data!
*/

#ifdef __cplusplus
}
#endif

/*
 Congratulations on making it this far - now might be a good time to check out sxml_test.c for an example of using SXML.
*/

#endif /* _SXML_H_INCLUDED */
