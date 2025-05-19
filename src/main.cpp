#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <filesystem>
#include <cstdlib>

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

    if (ch == '\\' && !in_single_quote){ // backslash
      if (!is_double_quote){
        i++;
        if (input[i] == '\n') continue;
        else current.push_back(input[i]); 
      }
      else{
        char next = input[i + 1];
        if (next == '\n'){
          i++; // skip newline
          continue;
        }
        if (next == '\\' || next == '"' || next == '$'){
          current += next;
          i++;
        }
        else{
          current += '\\';
        }
      }
    }

    else if (ch == '\'' && !is_double_quote){ // single quote
      if (in_single_quote){
        tokens.push_back(current);
        current.clear();
      } else{
        if (i && (input[i - 1] == '\"' || input[i - 1] == '\'')){
          current = tokens.back();
          tokens.pop_back();
        }
      }
      in_single_quote = !in_single_quote;
    }

    else if (std::isspace(ch) && !in_single_quote && !is_double_quote){ // white space
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
    }

    else if (ch == '\"' && !in_single_quote){ // double quote
      if (is_double_quote){
        tokens.push_back(current);
        current.clear();
      } else{
        if (i && (input[i - 1] == '\"' || input[i - 1] == '\'')){
          current = tokens.back();
          tokens.pop_back();
        }
      }
      is_double_quote = !is_double_quote;
    }

    else {
      if (i > 0 && (input[i - 1] == '\"' || input[i - 1] == '\'') && !in_single_quote && !is_double_quote && current.size()==0){
        current = tokens.back();
        tokens.pop_back();
      }
      current += ch;
    }
  }
  // Add last token if any
  if (!current.empty())  tokens.push_back(current);

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
  if (get_path(command) != "")
    return true;
  return false;
}

std::string read_input(){
  std::cout << "$ ";
  input="";
  std::getline(std::cin, line);

  input += line;
  while (!line.empty() && line.back() == '\\')
  {
    if (line.size() >= 2 && line[line.size() - 2] == '\\')
    {
      break;
    }
    input += '\n';
    std::cout << "> ";
    if (!std::getline(std::cin, line))
      break;
    input += line;
  }
  return input;
}

bool perform_exit(){
  if (tokens.size() == 1 || (tokens.size() == 2 && tokens[1] == "0")) {
      return true; 
  } else {
      std::cerr << "Usage: exit [status]" << std::endl;
      return false;
  }
}


void perform_type(){
  for (int i = 1; i < tokens.size(); i++)
  {
    if (built_in_commands.find(tokens[i]) != built_in_commands.end())
      std::cout << tokens[i] << " is a shell builtin";
    else{
      std::string path = get_path(tokens[i]);
      if (path != "")
        std::cout << tokens[i] << " is " << path;
      else
        std::cout << tokens[i] << ": not found";
    }

    std::cout << std::endl;
  }
}

void perform_echo(){
  int n = tokens.size();
  if (n == 1)
    return;
  for (int i = 1; i < n; ++i)
    std::cout << tokens[i] << " ";
  std::cout << std::endl;
}

void perform_cd(){
  std::error_code ec;

  if (tokens.size() < 2 || tokens[1] == "~") std::filesystem::current_path(std::getenv("HOME"), ec);
  else std::filesystem::current_path(tokens[1], ec);

  if (ec.value() == 2) std::cout << "cd: " << tokens[1] << ": No such file or directory" << std::endl;
  
}

int main()
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  while (true)
  {
    std::string input = read_input();
    if(input.empty()) continue;
    tokens = parse_input(input);

    if(tokens[0]=="exit"){ if(perform_exit()) break;}
    else if (tokens[0] == "type") perform_type();
    else if (tokens[0] == "echo") perform_echo();
    else if (tokens[0] == "pwd") std::cout << std::filesystem::current_path().string() << std::endl;
    else if (tokens[0] == "cd") perform_cd();
    else if (valid_not_built_in_command(tokens[0]))
      std::system(input.c_str()); // .c_str() converts string to char*
    else
      std::cout << tokens[0] << ": command not found" << std::endl;
  }
}