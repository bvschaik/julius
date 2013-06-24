#ifndef LANGUAGE_H
#define LANGUAGE_H

int Language_load(const char *textfile, const char *helpfile);

const char *Language_getString(int group, int number);

#endif
