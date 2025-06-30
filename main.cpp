// thank you msdd for tips and tricks :-)

#include <iostream>
#include <map>
#include <chrono>
#include <thread>

const char spinner[] = {'|', '/', '-', '\\'};
bool spinning = 0;

void clear() {
    printf("\033[2J\033[1;1H");
}

void play_song(std::string path) {
    std::string command = "mpv --no-video --quiet \"" + path + "\" &";
    std::system(command.c_str());
}

void spin_wait(std::string Text) {
    int local_spin_index = 0;
    printf("\n");

    while (spinning) {
        std::cout << "\r " << Text << spinner[local_spin_index++] << std::flush;
        local_spin_index %= 4;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void download(std::string command) {
    FILE* pipe = popen(command.c_str(), "r");

    char buffer[1024];

    std::string line;
    
    spinning = true;
    std::thread spin(spin_wait, "Downloading...");

    if (!pipe) {
        std::cerr << "Song failed to download, perhaps you are missing yt-dlp?";
        return;
    }
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        line = buffer;
        if (line.find("[download]") != std::string::npos) {
            //printf("\r%s", line.substr(0, line.length() - 1).c_str());
            fflush(stdout);
        }
    }

    pclose(pipe);

    spinning = false;
    spin.join();
    
    printf("\nDownload complete.\n");
}

int to_number(std::string& line, int fail_safe = 1) {
    try {
        return std::stoi(line);
    } catch (const std::invalid_argument& e) {
        printf("unknown character, selecting 1");
    }

    return fail_safe;
}

std::string get_output(std::string command) {
    FILE* pipe = popen(command.c_str(), "r");
    std::string output;
    char buffer[512];

    spinning = true;
    std::thread spin(spin_wait, "Fetching Information...");

    while (fgets(buffer, sizeof(buffer), pipe) !=nullptr) {
        output += buffer;
    }

    pclose(pipe);
    spinning = false;
    spin.join();

    while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
        output.pop_back();
    }
    
    return output;
}

std::string get_input(std::string custom_prompt) {
    clear();

    std::string search_input;
    std::cout << custom_prompt;
    std::getline(std::cin >> std::ws, search_input);

    clear();
    return search_input;
}

std::map<int, std::string> search_query(std::string search_input) {
    std::map<int, std::string> query_results;
    std::string search_up_command = "yt-dlp --flat-playlist --print \"%(title)s || %(id)s\" \"ytsearch5:" + search_input + "\"";
    std::string song_selected;
    
    spinning = true;
    std::thread spin(spin_wait, "Searching...");

    FILE* pipe = popen(search_up_command.c_str(), "r");

    char buffer[512];
    int count = 0;
    while (fgets(buffer, sizeof(buffer), pipe) !=nullptr) {
        std::string line(buffer);

        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }

        size_t seperated = line.find(" || ");
        if (seperated != std::string::npos) {
            if (spinning) {
                spinning = false;
                clear();
            }

            count ++;

            std::string title = line.substr(0, seperated);
            std::string id = line.substr(seperated + 4);

            std::cout << "[" + std::to_string(count) + "] " + line << "\n";
            query_results[count] = id;
        }
    }
    
    if (spin.joinable()) spin.join();

    pclose(pipe);
    return query_results;
}

int main() {
    std::string file_format;
    std::string full_url;
    std::string command = "yt-dlp --embed-metadata --sleep-requests .25 --sleep-interval .25 --retry-sleep fragment:300 --output \"./songs/%(title)s.%(ext)s\" ";
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

        printf("Select number [or z to retry search]: ");
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

    std::string validate_url = "\"" + full_url + "\"";
    std::string song_path = "./songs/";

    song_path += get_output(("yt-dlp --get-title " + validate_url).c_str());
    song_path += (file_format_int == 1) ? ".mp3" : ".mp4";
    download((command + validate_url).c_str());
    
    if (song_path == "") {
        return 0;
    }

    std::string open_fzf = get_input("open the file with mpv? \n - [1] = yes \n - [2] = no \nInput: ");

    int open_fzf_int = to_number(open_fzf);

    if (file_format_int == 1) {
        std::string command = "mpv \"" + song_path + "\"";
        std::system(command.c_str());
    }

    return 0;
}
