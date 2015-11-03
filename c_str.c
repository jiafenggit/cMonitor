#include "c_str.h"

/**
 * [去除字符串中的'\t''\r''\n'' ']
 * @param 待处理字符串 
 * @return [函数是否成功执行]
 */
bool strip(char *str)
{
    char *buf = NULL;
    char *buf_head = NULL;
    char *str_head = NULL;
    if (*str == '\0' || str == NULL || strlen(str) == 0)
    {
        printf("Empty String\n");
        return false;
    }
    str_head = str;
    buf = (char *)calloc(strlen(str) + 1, sizeof(char));
    if (buf == NULL)
    {
        perror("calloc buf");
    }
    buf_head = buf;
    while (*str != '\0')
    {
        switch (*str) {
        case '\t':
            break;
        case '\r':
            break;
        case '\n':
            break;
        case ' ':
            break;
        default:
            *buf = *str;
            buf++;
            break;
        }
        str++;
    }
    *buf = '\0';
    buf = buf_head;
    str = str_head;
    memset(str,  '\0',  strlen(str) );
    memcpy(str, buf, strlen(buf));
    free(buf);
    buf = NULL;
    return true;
}

/**
 * [去除字符串中指定字符]
 * @param 待处理字符串
 * @param 需删除字符
 * @return [函数是否成功执行]
 */
bool split_strip(char *str, char *strip_chr)
{
    char *buf = NULL;
    char *buf_head = NULL;
    char *str_head = NULL;
    if (*str == '\0' || str == NULL || strlen(str) == 0)
    {
        printf("Empty String\n");
        return false;
    }
    str_head = str;
    buf = (char *)calloc(strlen(str) + 1, sizeof(char));
    if (buf == NULL)
    {
        perror("calloc buf");
        return false;
    }
    buf_head = buf;
    while (*str != '\0')
    {
    	if((*str) != (*strip_chr))
        {
            *buf = *str;
            buf++;
        }
        str++;
    }
    *buf = '\0';
    buf = buf_head;
    str = str_head;
    memset(str,  '\0',  strlen(str) );
    memcpy(str, buf, strlen(buf));
    free(buf);
    buf = NULL;
    return true;
}

/**
 * [去除字符串左侧指定字符串，遇到第一个未包含在删除字符串中的字符即停止]
 * @param 待处理字符串
 * @param 需删除字符串
 * @return [函数是否成功执行]
 */
bool l_strip(char *str, char *strip_chr)
{
	char *buf = NULL;
	char *str_head = NULL;
	int strip_char_count = 0;
	int strip_loction = 0;
	strip_char_count = strlen(strip_chr);
	str_head = str;
	buf = (char *)calloc(strlen(str) + 1, sizeof(char));
	if (buf == NULL)
	{
	    perror("calloc buf");
	    return false;
	}
	while(true)
	{
		char str_chr = *str_head;
		int index = 0;
		bool chr_equal = false;
		for (; index < strip_char_count; index++)
		{
			if (str_chr == strip_chr[index])
			{
				chr_equal = true;
				strip_loction++;
				break;
			}
		}
		if (!chr_equal)
		{
			if ((memncpy(buf, str, strip_loction, strlen(str) - strip_loction)) == false)
			{
				printf("Memncpu string error.\n");
				return false;
			}
			memset(str, 0, strlen(str));
			memcpy(str, buf, strlen(buf));
			free(buf);
			buf = NULL;
			return true;
		}
		str_head++;
	}
}

/**
 * [去除字符串右侧指定字符串，遇到第一个未包含在删除字符串中的字符即停止]
 * @param 待处理字符串
 * @param 需删除字符串
 * @return [函数是否成功执行]
 */
bool r_strip(char *str, char *strip_chr)
{
	char *buf = NULL;
	char *str_tail = NULL;
	int strip_char_count = 0;
	int strip_loction = 0;
	strip_char_count = strlen(strip_chr);
	str_tail = &str[strlen(str) -1];
	buf = (char *)calloc(strlen(str) + 1, sizeof(char));
	if (buf == NULL)
	{
	    perror("calloc buf");
	    return false;
	}
	while(true)
	{
		char str_chr = *str_tail;
		int index = 0;
		bool chr_equal = false;
		for (; index < strip_char_count; index++)
		{
			if (str_chr == strip_chr[index])
			{
				chr_equal = true;
				strip_loction++;
				break;
			}
		}
		if (!chr_equal)
		{
			if ((memncpy(buf, str, 0, strlen(str) - strip_loction)) == false)
			{
				printf("Memncpu string error.\n");
				return false;
			}
			memset(str, 0, strlen(str));
			memcpy(str, buf, strlen(buf));
			free(buf);
			buf = NULL;
			return true;
		}
		str_tail--;
	}
}

/**
 * [按照指定分隔符分割字符串，获取指定位置的字符串，字符串位置从0开始]
 * @param 需要的指定位置的字符串
 * @param 源字符串
 * @param 分割字符
 * @param 字符串位置
 * @return [函数是否成功执行]
 */
bool split(char *result_str, char * origin_str, char split_chr, int split_index)
{
    char *origin_str_buf = NULL;
    char *result_str_buf = NULL;
    char *origin_str_buf_head = NULL;
    int current_split_index = -1;
    int old_str_index = 0;
    int new_str_index = 0;

    if (*origin_str == '\0' || origin_str == NULL || strlen(origin_str) == 0)
    {
        perror("Empty String");
        return false;
    }
    origin_str_buf = (char *)calloc(strlen(origin_str) + 1, sizeof(char));
    memcpy(origin_str_buf, origin_str, strlen(origin_str));
    origin_str_buf_head = origin_str_buf;
    while('\0' != *origin_str_buf)
    {
	if (split_chr == *origin_str_buf )
        {
	    if (new_str_index - old_str_index == 0)
	    {
		    old_str_index++;
		    new_str_index++;
		    origin_str_buf++;
		    continue;
	    }
	    free(result_str_buf);
	    result_str_buf  = NULL;
	    result_str_buf = (char *)calloc(new_str_index - old_str_index + 1, sizeof(char));
	    memncpy(result_str_buf, origin_str, old_str_index, new_str_index - old_str_index );
	    if(split_strip(result_str_buf, &split_chr) == false)
            {
                printf("String strip error.\n");
                free(result_str_buf);
                result_str_buf = NULL;
                free(origin_str_buf_head);
                origin_str_buf_head = NULL;
                return false;
            }
	    new_str_index++;
	    old_str_index = new_str_index;
            current_split_index++;
            if(current_split_index == split_index)
            {
                memcpy(result_str, result_str_buf, strlen(result_str_buf));
                free(result_str_buf);
                result_str_buf = NULL;
                free(origin_str_buf_head);
                origin_str_buf_head = NULL;
                return true;
            }
            else
            {
                free(result_str_buf);
                result_str_buf = NULL;
            }
        }
        else
        {
            new_str_index++;
        }
        origin_str_buf++;
    }
    if(current_split_index == -1 && old_str_index == new_str_index)
    {
        printf("Can't find split char.\n");
        free(origin_str_buf_head);
        origin_str_buf_head = NULL;
        return false;
    }
    else
    {
        result_str_buf = (char *)calloc(new_str_index - old_str_index + 1, sizeof(char));
        memncpy(result_str_buf, origin_str, old_str_index, new_str_index - old_str_index);
	if(split_strip(result_str_buf, &split_chr) == false)
        {
            printf("String strip error.\n");
            free(origin_str_buf_head);
            origin_str_buf_head = NULL;
            free(result_str_buf);
            result_str_buf = NULL;
            return false;
        }
        current_split_index++;
        if(current_split_index == split_index)
        {
            memcpy(result_str, result_str_buf, strlen(result_str_buf));
            free(origin_str_buf_head);
            origin_str_buf_head = NULL;
            free(result_str_buf);
            result_str_buf = NULL;
            return true;
        }
        else
        {
            printf("Split index error.\n");
            free(origin_str_buf_head);
            origin_str_buf_head = NULL;
            free(result_str_buf);
            result_str_buf = NULL;
            return false;
        }
    }
}

/**
 * [按照指定分隔符分割字符串，获取指定位置的字符串，字符串位置从0开始]
 * @param 源字符串
 * @param 分割字符
 * @param 字符串位置
 * @return [需要的指定位置的字符串]
 */
char *str_split(char * origin_str, char split_chr, int split_index)
{
	char *result_str = NULL;
	result_str = (char *)calloc(4096, sizeof(char));
	if (split(result_str, origin_str, split_chr, split_index) == false)
	{
		return NULL;
	}
	else
	{
		return result_str;
	}
}

/**
 * [从源字符串指定位置开始拷贝指定个数字符]
 * @param 拷贝结果
 * @param 源字符串
 * @param 拷贝开始位置
 * @param 拷贝个数
 * @return [函数是否成功执行]
 * eg：memncpy(reply, "hello world", 1, 3); 
 *     reply : "ell"
 */
bool memncpy(char *result_str, char *origin_str, int start_index, int cpy_num)
{
    int curent_cpy_num = 0;
    char *result_str_head = NULL;

    if (*origin_str == '\0' || origin_str == NULL || strlen(origin_str) == 0)
    {
	printf("Empty String.\n");
        return false;
    }
    if (start_index >= strlen(origin_str))
    {
	    printf("Error Start Index.\n");
	    return false;
    }
    result_str_head = result_str;
    for (; curent_cpy_num < cpy_num; curent_cpy_num++)
    {
        if (origin_str[start_index + curent_cpy_num] == '\0')
        {
            //printf("cpy_num is too long\n");
            break;
        }
        *result_str = origin_str[start_index + curent_cpy_num];
        result_str++;
    }
   *result_str = '\0';
    result_str = result_str_head;
    return true;
}
