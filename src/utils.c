#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/**
 * Basic utility function to trim a string, it trims leading and traling
 * whitespace characters. It returns the modified string which is trimmed.
 */
char *trim(char *str)
{
	char *end;

	while (isspace((unsigned char)*str))
		str++;

	if (*str == '\0')
		return NULL;

	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end))
		end--;

	end[1] = '\0';
	return str;
}

/**
 * This functions splits a string on the basis of the provided delimeter,
 * the integer `ctr` holds the count of the elements in the result we
 * returned. Original string is not modified in this function.
 */
char **str_split(char *str, char *delim, int *ctr)
{
	char *tmp_str = str;
	size_t len = strlen(str);
	size_t delim_len = strlen(delim);

	for (int i = 0;
	     (i <= len - delim_len) && !strncmp(tmp_str, delim, delim_len);
	     i++) {
		tmp_str += delim_len;
	}

	len = strlen(tmp_str);

	if (len > delim_len) {
		for (int i = len - delim_len;
		     i >= 0 && !strncmp(tmp_str + i, delim, delim_len); i++) {
			tmp_str[i] = '\0';
		}
	}

	char *new_str = tmp_str;

	char **result = NULL;
	size_t count = 1;

	int dup_flag = 0;
	while (strlen(tmp_str) >= delim_len) {
		if (!strncmp(tmp_str, delim, delim_len)) {
			tmp_str += delim_len;
			if (dup_flag)
				continue;

			count++;
			dup_flag = 1;
		} else {
			tmp_str++;
			dup_flag = 0;
		}
	}

	result = malloc(sizeof(char *) * (count + 1));

	if (result) {
		size_t idx = 0;
		char *token = strtok(new_str, delim);

		while (token) {
			*(result + idx++) = strdup(trim(token));
			token = strtok(NULL, delim);
		}

		*(result + idx) = NULL;
	}

	*ctr = count;

	return result;
}
