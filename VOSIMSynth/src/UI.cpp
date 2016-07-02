#include "UI.h"
#include <map>
#include <array>
#include <Commdlg.h>

namespace synui
{
	array<char, 8> utf8(int c) {
		array<char, 8> seq;
		int n = 0;
		if (c < 0x80)
			n = 1;
		else if (c < 0x800)
			n = 2;
		else if (c < 0x10000)
			n = 3;
		else if (c < 0x200000)
			n = 4;
		else if (c < 0x4000000)
			n = 5;
		else if (c <= 0x7fffffff)
			n = 6;
		seq[n] = '\0';
		switch (n) {
		case 6: seq[5] = 0x80 | (c & 0x3f);
			c = c >> 6;
			c |= 0x4000000;
		case 5: seq[4] = 0x80 | (c & 0x3f);
			c = c >> 6;
			c |= 0x200000;
		case 4: seq[3] = 0x80 | (c & 0x3f);
			c = c >> 6;
			c |= 0x10000;
		case 3: seq[2] = 0x80 | (c & 0x3f);
			c = c >> 6;
			c |= 0x800;
		case 2: seq[1] = 0x80 | (c & 0x3f);
			c = c >> 6;
			c |= 0xc0;
		case 1: seq[0] = c;
		}
		return seq;
	}

	int __nanogui_get_image(NVGcontext* ctx, const string& name, uint8_t* data, uint32_t size) {
		static map<string, int> iconCache;
		auto it = iconCache.find(name);
		if (it != iconCache.end())
			return it->second;
		int iconID = nvgCreateImageMem(ctx, 0, data, size);
		if (iconID == 0)
			throw runtime_error("Unable to load resource data.");
		iconCache[name] = iconID;
		return iconID;
	}

	vector<pair<int, string>> loadImageDirectory(NVGcontext* ctx, const string& path) {
		vector<pair<int, string>> result;
#if !defined(_WIN32)
		DIR *dp = opendir(path.c_str());
		if (!dp)
			throw std::runtime_error("Could not open image directory!");
		struct dirent *ep;
		while ((ep = readdir(dp))) {
			const char *fname = ep->d_name;
#else
		WIN32_FIND_DATA ffd;
		string searchPath = path + "/*.*";
		HANDLE handle = FindFirstFileA(searchPath.c_str(), &ffd);
		if (handle == INVALID_HANDLE_VALUE)
			throw runtime_error("Could not open image directory!");
		do {
			const char* fname = ffd.cFileName;
#endif
			if (strstr(fname, "png") == nullptr)
				continue;
			string fullName = path + "/" + string(fname);
			int img = nvgCreateImage(ctx, fullName.c_str(), 0);
			if (img == 0)
				throw runtime_error("Could not open image data!");
			result.push_back(
				make_pair(img, fullName.substr(0, fullName.length() - 4)));
#if !defined(_WIN32)
		}
		closedir(dp);
#else
		} while (FindNextFileA(handle, &ffd) != 0);
		FindClose(handle);
#endif
		return result;
	}

	Color colorFromHSL(float H, float S, float L) {
		float chroma = (1 - abs(2 * L - 1)) * S;
		float Hprime = fmod(H, 1.0) * 6.0;
		float X = chroma * (1 - abs(fmod(Hprime, 2.0f) - 1));
		Vector3f rgb;
		if (Hprime < 1) {
			rgb = {chroma,X,0};
		} else if (Hprime < 2) {
			rgb = {X,chroma,0};
		} else if (Hprime < 3) {
			rgb = {0,chroma,X};
		} else if (Hprime < 4) {
			rgb = {0,X,chroma};
		} else if (Hprime < 5) {
			rgb = {X,0,chroma};
		} else {
			rgb = {chroma,0,X};
		}
		rgb += Vector3f::Ones() * (L - 0.5f * chroma);
		return Color(rgb);
	}

#if !defined(__APPLE__)
	string file_dialog(const vector<pair<string, string>>& filetypes, bool save) {
#define FILE_DIALOG_MAX_BUFFER 1024
#if defined(_WIN32)
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		char tmp[FILE_DIALOG_MAX_BUFFER];
		ofn.lpstrFile = tmp;
		ZeroMemory(tmp, FILE_DIALOG_MAX_BUFFER);
		ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
		ofn.nFilterIndex = 1;

		string filter;

		if (!save && filetypes.size() > 1) {
			filter.append("Supported file types (");
			for (size_t i = 0; i < filetypes.size(); ++i) {
				filter.append("*.");
				filter.append(filetypes[i].first);
				if (i + 1 < filetypes.size())
					filter.append(";");
			}
			filter.append(")");
			filter.push_back('\0');
			for (size_t i = 0; i < filetypes.size(); ++i) {
				filter.append("*.");
				filter.append(filetypes[i].first);
				if (i + 1 < filetypes.size())
					filter.append(";");
			}
			filter.push_back('\0');
		}
		for (auto pair : filetypes) {
			filter.append(pair.second);
			filter.append(" (*.");
			filter.append(pair.first);
			filter.append(")");
			filter.push_back('\0');
			filter.append("*.");
			filter.append(pair.first);
			filter.push_back('\0');
		}
		filter.push_back('\0');
		ofn.lpstrFilter = filter.data();

		if (save) {
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			if (GetSaveFileNameA(&ofn) == FALSE)
				return "";
		} else {
			ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileNameA(&ofn) == FALSE)
				return "";
		}
		return string(ofn.lpstrFile);
#else
		char buffer[FILE_DIALOG_MAX_BUFFER];
		std::string cmd = "/usr/bin/zenity --file-selection ";
		if (save)
			cmd += "--save ";
		cmd += "--file-filter=\"";
		for (auto pair : filetypes)
			cmd += "\"*." + pair.first + "\" ";
		cmd += "\"";
		FILE *output = popen(cmd.c_str(), "r");
		if (output == nullptr)
			throw std::runtime_error("popen() failed -- could not launch zenity!");
		while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
			;
		pclose(output);
		std::string result(buffer);
		result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
		return result;
#endif
	}
#endif
}
