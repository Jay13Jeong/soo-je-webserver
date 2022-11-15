#include <iostream>

struct s_split {
private:
	int	ft_s_count(char const *s, char c)
	{
		size_t		count;

		count = 0;
		while (*s)
		{
			if (*s != c)
			{
				count++;
				while (*s && (*s != c))
					s++;
			}
			else
				s++;
		}
		return (count);
	}

	static char	**ft_check_free(char **dst, int dst_index)
	{
		int		index;

		index = 0;
		while (index < dst_index)
		{
			free(dst[index]);
			index++;
		}
		free(dst);
		dst = NULL;
		return (dst);
	}

	size_t	ft_strlen(const char *s)
	{
		size_t	index;

		index = 0;
		while (s[index] != '\0')
			index++;
		return (index);
	}

	size_t	ft_strlcpy(char *dst, const char *src, size_t dstsize)
	{
		unsigned int	index;
		unsigned int	src_strlen;

		if (!src)
			return (0);
		src_strlen = ft_strlen(src);
		index = 0;
		while (index + 1 < dstsize)
		{
			if (index == src_strlen)
				break ;
			dst[index] = src[index];
			index++;
		}
		if (dstsize != 0)
			dst[index] = '\0';
		return (src_strlen);
	}

	char	**ft_split_trim(char **s)
	{
		int str_count;
		int i;
		char *temp;
		char **dst;

		i = 0;
		str_count = 0;
		while (s[i] != NULL)
		{
			if (ft_strlen(s[i]) != 0)
				str_count++;
			i++;
		}




		i = 0;
		while (s[i] != NULL)
		{
			free(s[i]);
			i++;
		}
		free(s);
		return (dst);
	}

public:
	char	**ft_split(char const *s, char c)
	{
		char		**dst;
		char		*start;
		int			dst_index;

		dst_index = 0;
		dst = (char **)malloc(sizeof(char *) * (ft_s_count(s, c) + 1));
		if (!dst)
			return (NULL);
		while (*s)
		{
			if (*s != c)
			{
				start = (char *)s;
				while (*s && (*s != c))
					s++;
				dst[dst_index] = (char *)malloc(sizeof(char) * (s - start + 1));
				if (!dst[dst_index])
					return (ft_check_free(dst, dst_index));
				ft_strlcpy(dst[dst_index], start, s - start + 1);
				dst_index++;
			}
			else
				s++;
		}
		return (dst[dst_index] = NULL, ft_split_trim(dst));
	}
};