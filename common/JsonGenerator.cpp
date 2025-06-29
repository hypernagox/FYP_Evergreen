#include "pch.h"
#include "JsonGenerator.h"
#include "PathManager.h"
#include "json.hpp"

namespace Common
{
    static void CapitalizeEachWord(std::string& str) noexcept
    {
        bool new_word = true;
        for (char& c : str)
        {
            if (std::isspace(static_cast<unsigned char>(c))) {
                new_word = true;
            }
            else if (new_word) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                new_word = false;
            }
        }
    }

    static void GenerateJsonItem(const std::wstring_view json_path_w)
    {
        std::filesystem::path json_path{ json_path_w };
        nlohmann::ordered_json root;

        {
            std::ifstream input_stream{ json_path };
            if (input_stream.is_open()) {
                try {
                    input_stream >> root;
                }
                catch (...) {
                    root = nlohmann::ordered_json::object();
                }
            }
        }

        std::filesystem::path item_root_dir = json_path.parent_path().parent_path() / L"item";

        for (const auto& entry : std::filesystem::directory_iterator{ item_root_dir })
        {
            if (!entry.is_directory())
                continue;

            std::string folder_name = entry.path().filename().string();
            std::string item_name = folder_name;

            std::ranges::transform(item_name.begin(), item_name.end(), item_name.begin(),
                [](unsigned char c) noexcept { return static_cast<char>(std::tolower(c)); });
            item_name.erase(std::remove(item_name.begin(), item_name.end(), ' '), item_name.end());

            CapitalizeEachWord(folder_name);

            std::string dropitem;
            std::string diffuse;

            for (const auto& file : std::filesystem::directory_iterator{ entry.path() })
            {
                if (!file.is_regular_file())
                    continue;

                const std::string filename = file.path().filename().string();
                const std::string extension = file.path().extension().string();

                if (extension == ".yms")
                {
                    dropitem = "item\\" + item_name + "\\" + filename;
                }
                else if (extension == ".png")
                {
                    diffuse = "item\\" + item_name + "\\" + filename;
                }
            }

            if (dropitem.empty() && diffuse.empty())
                continue;

            if (root.contains(folder_name))
            {
                if (!dropitem.empty())
                    root[folder_name]["DropitemResource"] = dropitem;
                if (!diffuse.empty())
                    root[folder_name]["DropitemResourceDiffuse"] = diffuse;
            }
            else
            {
                root[folder_name] = nlohmann::ordered_json{
                    { "Name", "" },
                    { "Description", "" },
                    { "Category", "" },
                    { "Subcategory", "" },
                    { "Icon", "" },
                    { "DropitemResource", dropitem },
                    { "DropitemResourceDiffuse", diffuse }
                };
            }
        }

        std::ofstream output_stream{ json_path };
        if (output_stream.is_open())
        {
            output_stream << root.dump(4);
        }
    }

    bool JsonGenerator::GenerateJson(const std::wstring_view path) noexcept
    {
        const std::unordered_map<std::wstring, std::pair<std::function<void(const std::wstring_view)>, std::wstring>> generator =
        {
            { L"item", { GenerateJsonItem, RESOURCE_PATH(path) + L"\\json\\item.json" } }
        };

        for (const auto& [sub, pair] : generator)
        {
            std::filesystem::path root_dir = RESOURCE_PATH(path) + L"\\" + sub;

            if (!std::filesystem::exists(root_dir))
                continue;

            pair.first(pair.second);
        }

        return true;
    }
}