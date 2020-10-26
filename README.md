# from_chars
A slight variation from std::from_chars.

 This header assumes no knowledge of input, format is detected during the parse.

Parses integers (while ignoring specified characters) in several forms:
* binary:      0[bB][01]+
* octal:       0[01234567]*
* decimal:     [123456789][0123456789]*
* hexidecimal: 0[xX][0123456789ABCDEFabcdef]+

TODO:
floating points
