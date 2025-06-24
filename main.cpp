#include <iostream>
#include <map>

void clear() {
    std::printf("\033[2J\033[1;1H");
}

int to_number(std::string& line, int fail_safe = 1) {
    try {
        return std::stoi(line);
    } catch (const std::invalid_argument& e) {
        std::printf("Unknown character, selecting 1");
    }

    return fail_safe;
}

auto get_input(std::string custom_prompt) {
    clear();

    std::string search_input;
    std::cout << custom_prompt;
    std::getline(std::cin >> std::ws, search_input);

    clear();
    return search_input;
}

auto search_query(std::string search_input) {
    std::map<int, std::string> query_results;
    std::string search_up_command = "yt-dlp --flat-playlist --print \"%(title)s || %(id)s\" \"ytsearch5:" + search_input + "\"";
    std::string song_selected;

    auto pipe = popen(search_up_command.c_str(), "r");

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
            query_results[count] = id;
        }
    }

    pclose(pipe);
    return query_results;
}

int main() {
    std::string file_format;
    std::string full_url;
    std::string command = "yt-dlp --embed-metadata --sleep-requests .5 --sleep-interval .5 --retry-sleep fragment:300 ";
    std::string look_up_option = get_input("Look Up Method? \n - [1] = url, \n - [2] = ytsearch \n - [x] = exit program \nInput: ");

    if (look_up_option == "x") {
        return 0;
    }

    int search_option = to_number(look_up_option);

    if (search_option == 1) {
        full_url = get_input("Input URL: ");
    } else {
        search_again:

        std::string search_input = get_input("Search: ");
        std::map<int, std::string> query_results = search_query(search_input);

        std::cout << "Select number [or z to retry search]: ";
        std::getline(std::cin >> std::ws, search_input);

        if (search_input == "z") {
            goto search_again;
        }

        int selected_choice = to_number(search_input);
        full_url = "https://youtube.com/watch?v=" + query_results[selected_choice];
    }

    file_format = get_input("file format? \n - [1] = mp3 \n - [2] = mp4 \nInput: ");

    int file_format_int = to_number(file_format);

    if (file_format_int == 2) {
        command += "-f 'bestvideo[ext=mp4]+bestaudio[ext=m4a]/mp4' ";
    } else {
        command += "-x --audio-format mp3 ";
    }

    std::string execute_command = command + "\"" + full_url + "\"";
    int out = std::system(execute_command.c_str());

    if (out != 0) {
        std::cerr << "Song failed to download, Error code: " + std::to_string(out);
        return 1;
    }

    return 0;
}
