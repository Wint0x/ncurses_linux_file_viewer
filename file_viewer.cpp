#include <ncurses.h>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

bool check_path(const fs::path f_path)
{
	return fs::is_directory(f_path);
}

void iter_files(const fs::path f_path, std::vector<fs::path>& f_vec)
{
	fs::directory_iterator dir_it{f_path, fs::directory_options::skip_permission_denied};

	for (const auto& dir_entry : dir_it)
		f_vec.emplace_back(dir_entry.path());
}

void sort_dir_first(std::vector<fs::path>& f_vec)
{
	std::partition(f_vec.begin(), f_vec.end(), check_path);
}

int main(int argc, const char **argv)
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);

	int yMax, xMax, yWinMax, xWinMax;
	getmaxyx(stdscr, yMax, xMax);
	
	// Color check
	if (!has_colors())
	{
		printw("Your terminal does not support colors.");
		return -1;
	}

	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
	
	attr_t dir_attr = COLOR_PAIR(4) | A_BOLD;

	// Arguments
	if (argc < 2)
	{
		attron(COLOR_PAIR(1));
		printw("Please provide a path!");
		attroff(COLOR_PAIR(1));

		getch();
		endwin();

		return 1;
	}

	size_t s_arg_name = std::string(argv[0]).size();
	
	std::string arg_path{argv[1]};
	fs::path path(arg_path);

	std::vector<fs::path> files;

	if (!check_path(path))
	{
		attron(COLOR_PAIR(3));
		printw("Path \"%s\" not found!", arg_path.c_str());
		attroff(COLOR_PAIR(3));
		
		getch();
		endwin();

		return 2;
	}

	attron(A_UNDERLINE);
	printw("%s", argv[0]);
	attroff(A_UNDERLINE);
	printw(" | ");
	mvchgat(0, s_arg_name + 3, -1, A_STANDOUT, 0, NULL);
	WINDOW *win = newwin(25, 25, 1, 0);
	getmaxyx(win, yWinMax, xWinMax);

	wattron(win, COLOR_PAIR(2));
	box(win, 0, 0);
	wattroff(win, COLOR_PAIR(2));
	
	// Fetch files
	iter_files(path, files);
	sort_dir_first(files);

	size_t file_count = files.size();

	// Add file count in window
	wattron(win, A_STANDOUT);
	mvwprintw(win, 1, 6, "%zu item/s...", file_count);
	wattroff(win, A_STANDOUT);

	// Display files
	int yPos = 2;
	std::string temp_file{""};
	for (auto item : files)
	{
		temp_file.clear();

		// Out of bounds prevention, replace file with "..."
		if (yPos >= yWinMax - 2)
		{
			mvwprintw(win, yPos, 1, ". . .");
			break;
		}

		// Fix X out of bound (name too long)
		temp_file = item.filename().string();
		if (temp_file.size() >= xWinMax)
		{
			temp_file = temp_file.substr(0, xWinMax - 6);
			temp_file.append("...");
		}

		// Check if dir or file
		if (check_path(item))
		{
			wattron(win, dir_attr);

			// Filename too long
			if (!temp_file.empty())
				mvwprintw(win, yPos, 1, "%s/", temp_file.c_str());

			else
				mvwprintw(win, yPos, 1, "%s/", item.filename().c_str());

			wattroff(win, dir_attr);
		}

		else
		{
			// Filename too long
			if (!temp_file.empty())
				mvwprintw(win, yPos, 1, "%s", temp_file.c_str());

			else
				mvwprintw(win, yPos, 1, "%s", item.filename().c_str());
		}

		yPos++;
	}

	refresh();

Exit:
	wgetch(win);
	endwin();
	
	return 0;
}
