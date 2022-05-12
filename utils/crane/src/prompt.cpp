#include "prompt.hpp"
#include "context.hpp"
#include <cctype>
#include <readline/history.h>
#include <readline/readline.h>
#include <string>

std::string generatePrompt(CraneContext *context) {
  /**
   * The expected prompt should be clean and a bit colorful:
   *
   * {cyan}crane {magenta}(openFile)? {white if didNotFail else red}>{reset}
   */

  std::string out;

  // \33[2K\r
  out += "\33[2K\r";

  // {cyan}crane
  out += std::string(kColorCyan) + "crane ";

  // {magenta}{openFile}?
  if (context->openedFile != nullptr) {
    out += std::string(kColorMagenta) + "(" + context->openedFile->alias + ") ";
  }

  out += context->lastCommandResult != 0 ? kColorRed : kColorWhite;
  out += "> " + std::string(kColorReset);

  return out;
}

CraneCommand *CraneCommand::parseCommand(std::string buffer) {
  CraneCommand *command = new CraneCommand("", {});
  std::string argumentBuffer = "";
  bool escapeNext = false;
  bool inString = false;
  bool hasCommandName = false;
  char lastQuote = '\0';
  char currentChar = '\0';
  int bufferLength = buffer.length();
  int lastQuoteIndex = 0;
  int i = 0;

  while (bufferLength) {
    currentChar = buffer[i++];

    if (escapeNext) {
      argumentBuffer.push_back(currentChar);
      continue;
    }

    if (currentChar == '\\' && !escapeNext) {
      escapeNext = true;
      continue;
    }

    if (currentChar == '"' || currentChar == '\'') {
      if (!inString) {
        inString = true;
        lastQuoteIndex = i;
        lastQuote = currentChar;
        continue;
      } else if (currentChar == lastQuote) {
        inString = false;

        if (!hasCommandName) {
          command->name = argumentBuffer;
          hasCommandName = true;
        } else {
          command->arguments.push_back(argumentBuffer);
        }
        argumentBuffer.clear();
      } else {
        argumentBuffer.push_back(currentChar);
      }
    }

    if (isspace(currentChar) && !inString) {
      if (!hasCommandName) {
        command->name = argumentBuffer;
        hasCommandName = true;
      } else {
        command->arguments.push_back(argumentBuffer);
      }
      argumentBuffer.clear();

      continue;
    }

    argumentBuffer.push_back(currentChar);
  }

  if (inString) {
    printf("Expected closing quote (%c) to string at %d:\n", lastQuote,
           lastQuoteIndex);
    printf("%s\n", buffer.c_str());
    printf("%s^\n", std::string(lastQuoteIndex - 1, ' ').c_str());
    return nullptr;
  }

  return command;
}

CraneCommand *CraneCommand::fromUser(CraneContext *context) {
  std::string prompt = generatePrompt(context);
  char *commandBuffer = readline(prompt.c_str());

  if (!commandBuffer) {
    return nullptr;
  }
  add_history(commandBuffer);

  return CraneCommand::parseCommand(std::string(commandBuffer));
}
