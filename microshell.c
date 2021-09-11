#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef struct s_data
{
	int quantity;
	int type;
	int pipes[2];
	char **argument;
}	t_data;

int	ft_strlen (char *str)
{
	int i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_copy (char *orig, char *copy)
{
	int i = 0;
	while (orig[i])
	{
		copy[i] = orig[i];
		i++;
	}
	copy[i] = '\0';
}

int	ft_fatal_error()
{
	write(2, "error: fatal\n", ft_strlen("error: fatal"));
	exit (1);
	return (1);
}

int	ft_data_count(int argc, char **argv)
{
	int count = 0;
	int log = 0;
	for (int i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], ";") == 0) || (strcmp(argv[i], "|") == 0))
			log = 0;
		else
		{
			if (log == 0)
				count++;
			log = 1;
		}
	}
	return (count);
}

int ft_data_quantity(int argc, char **argv, int *start, int *pipe_log)
{
	int quantity = 0;
	*pipe_log = 0;
	if (strcmp(argv[*start], "|") == 0)
		*start = *start + 1;
	while (*start < argc)
	{
		if (strcmp(argv[*start], ";") != 0)
			break ;
		*start = *start + 1;
	}
	while (*start < argc)
	{
		if (strcmp(argv[*start], "|") == 0)
		{
			*pipe_log = 1;
			break ;
		}
		if (strcmp(argv[*start], ";") == 0)
			break ;
		*start = *start + 1;
		quantity++;
	}
	return (quantity);
}

int show_error(char *str)
{
	write(2, str, ft_strlen(str));
	return (1);
}

int	ft_data_start_execve(t_data *data, char **env, int i, int count)
{
	int ret = 0;
	int status = 0;
	pid_t pid;
	int pipe_open = 0;
	if ((data[i].type == 1) || ((i > 0) && (data[i - 1].type == 1)))
	{
		pipe_open = 1;
		if (pipe(data[i].pipes))
			return (ft_fatal_error());
	}
	pid = fork();
	if (pid < 0)
		return (ft_fatal_error());
	if (pid == 0)
	{
		if ((data[i].type == 1) && (dup2(data[i].pipes[1], 1) < 0))
			return (ft_fatal_error());
		if ((i > 0) && (data[i - 1].type == 1) && (dup2(data[i - 1].pipes[0], 0) < 0))
			return (ft_fatal_error());
		if ((ret = execve(data[i].argument[0], data[i].argument, env)) < 0)
		{
			show_error("error: cannot execute ");
			show_error(data[i].argument[0]);
			show_error("\n");
		}
		exit (ret);
	}
	else
	{
		waitpid(pid, &status, 0);
		if (pipe_open)
		{
			close(data[i].pipes[1]);
			if (data[i].type == 2)
				close(data[i].pipes[0]);
		}
		if ((i > 0) && (data[i - 1].type == 1))
			close(data[i - 1].pipes[0]);
		if (WIFEXITED(status))
			ret = WEXITSTATUS(status);
	}
	return (ret);
}

int ft_start(t_data *data, int count, char **env)
{
	int ret = 0;
	for (int i = 0; i < count; i++)
	{
		if ((strcmp (data[i].argument[0], "cd") == 0) && data[i].quantity == 1)
			ret = show_error("error: cd: bad arguments\n");
		else if (strcmp (data[i].argument[0], "cd") == 0)
		{
			if (chdir(data[i].argument[1]))
			{
				ret = show_error("error: cd: cannot change directory to ");
				show_error(data[i].argument[1]);
				show_error("\n");
			}
		}
		else
			ret = ft_data_start_execve(data, env, i, count);
	}
	return (ret);
}

int main(int argc, char **argv, char **env)
{
	if (argc < 2)
		return (0);
	int ret = 0;
	int count = ft_data_count(argc, argv);
	t_data *data = malloc(sizeof(t_data) * count);
	if (data == NULL)
		ft_fatal_error();
	int start = 1; 
	for (int i = 0; i < count; i++)
	{
		data[i].quantity = ft_data_quantity(argc, argv, &start, &data[i].type);
		if ((i > 0) && (data[i].type == 0) && (data[i - 1].type == 1))
			data[i].type = 2;
		data[i].argument = malloc(sizeof(char *) * (data[i].quantity + 1));
		if (data[i].argument == NULL)
			ft_fatal_error();
		int j = 0;
		for (; j < data[i].quantity; j++)
		{
			data[i].argument[j] = malloc(sizeof(char) * (ft_strlen(argv[start - data[i].quantity + j]) + 1));
			if (data[i].argument[j] == NULL)
				ft_fatal_error();
			ft_copy(argv[start - data[i].quantity + j], data[i].argument[j]);
		}
		data[i].argument[j] = NULL;
	}
	ret = ft_start(data, count, env);
	return (ret);
}
