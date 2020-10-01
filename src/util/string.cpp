#include <util/string.hpp>
#include <types.hpp>


void *memcpy (void *dest, const void *src, usize n);
void memset (void *s, usize n, u8 c);
usize strlen (const char *__restrict__ str);
int strcmp (const char *__restrict__ str1, const char *__restrict__ str2);
int strncmp (const char *__restrict__ str1, const char *__restrict__ str2, usize n);
void revstr (char *__restrict__ str);
u32 atoi (const char *__restrict__ str);
void itoa (char *__restrict__ str_out, u32 num);
void itoa_hex (char *__restrict__ str_out, u32 num);
char itoc (u32 num);


void *memcpy (void *dest, const void *src, usize n)
{
	u8 *temp_dest = (u8 *) dest;
	const u8 *temp_src = (const u8 *) src;
	for (usize i = 0; i < n; i ++)
	{
		temp_dest[i] = temp_src[i];
	}
	return dest;
}

void memset (void *s, usize n, u8 c)
{
	u8 *ss = (u8 *) s;
	for (usize i = 0; i < n; i ++)
	{
		ss[i] = c;
	}
}

usize strlen (const char *__restrict__ str)
{
	usize out = 0;
	while (str[out])
	{
		out ++;
	}
	return out;
}

int strcmp (const char *__restrict__ str1, const char *__restrict__ str2)
{
	for (usize i = 0; str1[i] && str2[i]; i ++)
	{
		char temp = str1[i] - str2[i];
		if (temp)
		{
			return temp;
		}
	}
	return 0;
}

int strncmp (const char *__restrict__ str1, const char *__restrict__ str2, usize n)
{
	for (usize i = 0; str1[i] && str2[i] && i < n; i ++)
	{
		char temp = str1[i] - str2[i];
		if (temp)
		{
			return temp;
		}
	}
	return 0;
}

void revstr (char *__restrict__ str)
{
	usize len = strlen (str);
	for (usize i = 0; i < len / 2; i ++)
	{
		char temp = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = temp;
	}
}

u32 atoi (const char *__restrict__ str)
{
	int out = 0;
	int num = powi (10, strlen (str) - 1);

	for (usize i = 0; str[i]; i ++)
	{
		switch (str[i])
		{
			case '0':
				out += 0;
				break;
			case '1':
				out += 1 * num;
				break;
			case '2':
				out += 2 * num;
				break;
			case '3':
				out += 3 * num;
				break;
			case '4':
				out += 4 * num;
				break;
			case '5':
				out += 5 * num;
				break;
			case '6':
				out += 6 * num;
				break;
			case '7':
				out += 7 * num;
				break;
			case '8':
				out += 8 * num;
				break;
			case '9':
				out += 9 * num;
				break;
			default:
				return 0;
		}
		num /= 10;
	}
	return out;
}

void itoa (char *__restrict__ str_out, u32 num)
{

}

void itoa_hex (char *__restrict__ str_out, u32 num)
{
	for (i8 i = 7; i >= 0 ; i --)
	{
		str_out[i] = itoc (num % 16);
		num >>= 4;
	}
	str_out[8] = '\0';
}

char itoc (u32 num)
{
	switch (num)
	{
		case 0:
			return '0';
		case 1:
			return '1';
		case 2:
			return '2';
		case 3:
			return '3';
		case 4:
			return '4';
		case 5:
			return '5';
		case 6:
			return '6';
		case 7:
			return '7';
		case 8:
			return '8';
		case 9:
			return '9';
		case 10:
			return 'a';
		case 11:
			return 'b';
		case 12:
			return 'c';
		case 13:
			return 'd';
		case 14:
			return 'e';
		case 15:
			return 'f';
		default:
			return '\0';
	}
}
