#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <filesystem>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

std::set<std::string> built_in_commands = {"echo", "type", "exit", "pwd", "cd"};
std::string path_env = std::getenv("PATH");
std::string input, line;
std::vector<std::string> tokens;

std::vector<std::string> parse_input(const std::string &input)
{
  std::vector<std::string> tokens;
  std::string current;
  bool in_single_quote = false, is_double_quote = false;

  for (size_t i = 0; i < input.size(); ++i)
  {
    char ch = input[i];

    if (ch == '\\' && !in_single_quote)
    {
      if (!is_double_quote)
      {
        i++;
        if (input[i] == '\n') continue;
        else current.push_back(input[i]);
      }
      else
      {
        char next = input[i + 1];
        if (next == '\n')
        {
          i++;
          continue;
        }
        if (next == '\\' || next == '"' || next == '$')
        {
          current += next;
          i++;
        }
        else
        {
          current += '\\';
        }
      }
    }
    else if (ch == '\'' && !is_double_quote)
    {
      if (in_single_quote)
      {
        tokens.push_back(current);
        current.clear();
      }
      else
      {
        if (i && (input[i - 1] == '\"' || input[i - 1] == '\''))
        {
          current = tokens.back();
          tokens.pop_back();
        }
      }
      in_single_quote = !in_single_quote;
    }
    else if (std::isspace(ch) && !in_single_quote && !is_double_quote)
    {
      if (!current.empty())
      {
        tokens.push_back(current);
        current.clear();
      }
    }
    else if (ch == '\"' && !in_single_quote)
    {
      if (is_double_quote)
      {
        tokens.push_back(current);
        current.clear();
      }
      else
      {
        if (i && (input[i - 1] == '\"' || input[i - 1] == '\''))
        {
          current = tokens.back();
          tokens.pop_back();
        }
      }
      is_double_quote = !is_double_quote;
    }
    else
    {
      if (i > 0 && (input[i - 1] == '\"' || input[i - 1] == '\'') &&
          !in_single_quote && !is_double_quote && current.size() == 0)
      {
        current = tokens.back();
        tokens.pop_back();
      }
      current += ch;
    }
  }
  if (!current.empty()) tokens.push_back(current);
  return tokens;
}

std::string get_path(std::string command)
{
  std::stringstream ss(path_env);
  std::string path;
  while (std::getline(ss, path, ':'))
  {
    std::string full_path = path + "/" + command;
    if (std::filesystem::exists(full_path))
    {
      return full_path;
    }
  }
  return "";
}

bool valid_not_built_in_command(std::string command)
{
  return get_path(command) != "";
}

std::string read_input()
{
  std::cout << "$ ";
  input = "";
  std::getline(std::cin, line);
  input += line;
  while (!line.empty() && line.back() == '\\')
  {
    if (line.size() >= 2 && line[line.size() - 2] == '\\') break;
    input += '\n';
    std::cout << "> ";
    if (!std::getline(std::cin, line)) break;
    input += line;
  }
  return input;
}

bool perform_exit()
{
  if (tokens.size() == 1 || (tokens.size() == 2 && tokens[1] == "0"))
  {
    return true;
  }
  else
  {
    std::cerr << "Usage: exit [status]" << std::endl;
    return false;
  }
}

void perform_type()
{
  for (int i = 1; i < tokens.size(); i++)
  {
    if (built_in_commands.find(tokens[i]) != built_in_commands.end())
      std::cout << tokens[i] << " is a shell builtin";
    else
    {
      std::string path = get_path(tokens[i]);
      if (!path.empty())
        std::cout << tokens[i] << " is " << path;
      else
        std::cout << tokens[i] << ": not found";
    }
    std::cout << std::endl;
  }
}

void perform_echo()
{
  for (int i = 1; i < tokens.size(); ++i)
    std::cout << tokens[i] << (i < tokens.size() - 1 ? " " : "");
  std::cout << std::endl;
}

void perform_cd()
{
  std::error_code ec;
  if (tokens.size() < 2 || tokens[1] == "~")
    std::filesystem::current_path(std::getenv("HOME"), ec);
  else
    std::filesystem::current_path(tokens[1], ec);

  if (ec.value() == 2)
    std::cout << "cd: " << tokens[1] << ": No such file or directory" << std::endl;
}

// ---------- Redirection Helper Functions ----------
int setup_output_redirection(const std::string &redirect_file, bool append_mode)
{
  if (redirect_file.empty()) return -1;

  int fd = open(redirect_file.c_str(), append_mode ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1)
  {
    perror("open");
    return -1;
  }

  int saved_stdout = dup(STDOUT_FILENO); //creates a copy of the file descriptor for stdout
  if (saved_stdout == -1)
  {
    perror("dup"); 
    close(fd);
    return -1;
  }

  if (dup2(fd, STDOUT_FILENO) == -1)
  {
    perror("dup2");
    close(fd);
    close(saved_stdout);
    return -1;
  }

  close(fd);
  return saved_stdout;
}

void restore_output(int saved_stdout)
{
  if (saved_stdout != -1)
  {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
}

int setup_error_redirection(const std::string &redirect_file, bool append_mode)
{
    if (redirect_file.empty()) return -1;

    int fd = open(redirect_file.c_str(),
                  append_mode ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC,
                  0644);
    if (fd == -1)
    {
        perror("open");
        return -1;
    }

    int saved_stderr = dup(STDERR_FILENO);
    if (saved_stderr == -1)
    {
        perror("dup");
        close(fd);
        return -1;
    }

    if (dup2(fd, STDERR_FILENO) == -1)
    {
        perror("dup2");
        close(fd);
        close(saved_stderr);
        return -1;
    }

    close(fd);
    return saved_stderr;
}

void restore_error(int saved_stderr)
{
    if (saved_stderr != -1)
    {
        dup2(saved_stderr, STDERR_FILENO);
        close(saved_stderr);
    }
}

// ---------- MAIN ----------
int main()
{
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true)
  {
    std::string input = read_input();
    if (input.empty()) continue;

    std::string redirect_file;
    bool append_mode = false, error_redirect= false;
    size_t redirect_pos = input.find('>');
    std::string command_part = input;

    if (redirect_pos != std::string::npos)
    {
      command_part = input.substr(0, redirect_pos);
      if(redirect_pos>0 && input[redirect_pos-1]=='1'){
        command_part = input.substr(0, redirect_pos-1);
      } else if(redirect_pos>0 && input[redirect_pos-1]=='2'){
        command_part = input.substr(0, redirect_pos-1);
        error_redirect = true;
      }
      
      if (input[redirect_pos + 1] == '>')
      {
        append_mode = true;
        redirect_pos++;
      }

      redirect_file = input.substr(redirect_pos + 1);
      redirect_file.erase(0, redirect_file.find_first_not_of(" \t"));
      redirect_file.erase(redirect_file.find_last_not_of(" \t") + 1);
    }

    tokens = parse_input(command_part);
    if (tokens.empty()) continue;

    const std::string &command = tokens[0];

    int saved_stdout = -1;
      if (!redirect_file.empty())
      {
        if(error_redirect)
          saved_stdout = setup_error_redirection(redirect_file, append_mode);
        else
          saved_stdout = setup_output_redirection(redirect_file, append_mode);
        if (saved_stdout == -1) continue;
      }

    if (built_in_commands.count(command))
    {

      if (command == "exit") {if (perform_exit()) break;}
      else if (command == "type") perform_type();
      else if (command == "echo") perform_echo();
      else if (command == "pwd") std::cout << std::filesystem::current_path().string() << std::endl;
      else if (command == "cd") perform_cd();
    }
    else if (valid_not_built_in_command(command))
    {
      pid_t pid = fork();
      if (pid == 0)
      {
        std::vector<char *> argv;
        for (auto &tok : tokens) argv.push_back(&tok[0]);
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        perror("execvp failed");
        exit(1);
      }
      else if (pid > 0)
      {
        waitpid(pid, nullptr, 0);
      }
      else
      {
        perror("fork failed");
      }
    }
    else
    {
      std::cout << command << ": command not found" << std::endl;
    }

    if (!redirect_file.empty()){
        if(error_redirect) restore_error(saved_stdout);
        else restore_output(saved_stdout);
    }
  }
}
