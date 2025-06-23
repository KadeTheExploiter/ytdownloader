#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <string>
#include <vector>

void clear() {
#ifdef _WIN32 // FUCK YOU WINDOWS, YOU HEARD ME FUCK YOU.
    std::system("cls");
#else
    std::system("clear");
#endif
}

int main() {
  std::string url;
  std::string command = "yt-dlp --embed-metadata ";
  std::string prompt;
  std::string option;

  clear();

  std::cout << "lookup method? \n - [1] = url, \n - [2] = ytsearch \n - [x] = exit program \nInput: ";
  std::cin >> option;

  int methodchosen = 0;
  if (option == "x") {
    return 0;
  }
  try {
    methodchosen = std::stoi(option);
  } catch (...) {
    std::cout << "\n sybau </3";
    return 0;
  }

  clear();

  if (methodchosen == 1) {
    std::cout << "Input URL:";
    std::cin >> url;
  } else if (methodchosen == 2) {
    std::string search = "";
    std::cout << "Search for:";
    std::getline(std::cin >> std::ws, search);

    if (search == "") {
      std::cout << "Search is empty, cfdasfsfdsfdwwhbwreancelling";
      return 0;
    }

    std::vector<std::pair<std::string, std::string>> searchresults;
    std::string lookupcommand = "yt-dlp --flat-playlist --print \"%(title)s || %(id)s\" \"ytsearch5:" + search + "\"";
    clear();

    FILE* pipe = popen(lookupcommand.c_str(), "r");
    char buffer[512];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), pipe) !=nullptr) {
      std::string line(buffer);

      if (!line.empty() && line.back() == '\n') {
        line.pop_back();
      }

      size_t seperated = line.find(" || ");
      if (seperated != std::string::npos) {
        count ++;

        std::string title = line.substr(0, seperated);
        std::string id = line.substr(seperated + 4);

        std::cout << "[" + std::to_string(count) + "] " + line << "\n";
        searchresults.emplace_back(title, id);
      }
    }

    pclose(pipe);

    // std::system(lookupcommand.c_str());
    std::string option = "";

    std::cout << "select a number to continue, otherwise just press x and go\n";
    std::cout << ":3 input:";

    std::cin >> option;
    if (option == "x") {
      return 0;
    }

    int selected = 0;
    try {
        selected = std::stoi(option);
    } catch (...) {
        std::cerr << "wow youre so quirky n fun!!!!!!! lolol\n";
        return 1;
    }

    clear();

    if (selected >= 1 and selected <= searchresults.size()) {
      url = "https://youtube.com/watch?v=" + searchresults[selected - 1].second;
    } else {
      std::cout << "think you are funny huh?";
      return 0;
    }
  }

  std::cout << "file format? \n - [1] = mp3 \n - [2] = mp4 \n - [x] = exit program \nInput: ";
  std::cin >> prompt;
  int selectedformat = 0;
  if (prompt == "x") {
    return 0;
  }

  try {
    selectedformat = std::stoi(prompt);
  } catch (...) {
      std::cerr << "try again bub, or else your bembership is in danger...\n";
      return 1;
  }
  if (selectedformat == 1) {
    command += "-x --audio-format mp3 ";
  } else if (selectedformat == 2) {
    command += "-f 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4' ";
  } else {
    std::cout << "\n whatever, fuck off and heres your mp3 \n";
    command += "-x --audio-format mp3 ";
  }

  clear();

  std::string info = "Selected:" + prompt + "\n";
  //std::cout << info;

  std::string execute_command = command + "\"" + url + "\"";
  int out = std::system(execute_command.c_str());

  if (out!=0) {
    std::cerr << "song failed to download idk why but hees a code numbar.." + std::to_string(out);
    return 1;
  }

  return 0;
}
