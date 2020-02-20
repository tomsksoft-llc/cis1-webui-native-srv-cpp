/*
 *    TomskSoft CIS1 WebUI
 *
 *   (c) 2019 TomskSoft LLC
 *   (c) Mokin Innokentiy [mia@tomsksoft.com]
 *
 */

#include "file_util.h"

#include <fstream>

namespace beast = boost::beast;

bool start_with(std::string_view src, std::string_view dst)
{
    return src.find(dst) == 0;
};

std::optional<std::string_view> mime_type_from_interpreter(
        std::string_view interpreter)
{
    if(start_with(interpreter, "bash"))     return "text/x-shellscript";
    if(start_with(interpreter, "sh"))       return "text/x-shellscript";
    if(start_with(interpreter, "java"))     return "text/x-java-source";
    if(start_with(interpreter, "python"))   return "text/x-python";
    if(start_with(interpreter, "php"))      return "text/php";
    if(start_with(interpreter, "perl"))     return "text/x-perl";
    if(start_with(interpreter, "php"))      return "text/php";

    return std::nullopt;
}

std::optional<std::string_view> mime_type_from_shebang(
        const std::filesystem::path& path)
{
    std::ifstream file(path);
    if(!file.is_open())
    {
        return std::nullopt;
    }

    std::string line;
    std::getline(file, line);

    const auto shebang = "#!";

    // replace it with start_with() when the C++20 will be used
    if(!start_with(line, shebang))
    {
        // the line doesn't start with Shebang directive
        return std::nullopt;
    }

    line.erase(0, std::strlen(shebang));

    std::stringstream ss;
    ss << line;

    std::optional<std::string_view> mime_type;
    std::string word;
    while(std::getline(ss, word, ' '))
    {
        const std::filesystem::path interpreter_path = word;
        const auto interpreter = interpreter_path.filename().string();

        if(interpreter == "env")
        {
            continue;
        }

        if(const auto type = mime_type_from_interpreter(interpreter); type)
        {
            // Set only not-nullopt
            mime_type = type;
        }
    }

    return mime_type;
}

beast::string_view mime_type(const std::filesystem::path& path)
{
    const beast::string_view default_mime_type = "application/text";

    if(!path.has_extension())
    {
        return mime_type_from_shebang(path).value_or(default_mime_type);
    }

    const beast::string_view ext = path.extension().string();

    using beast::iequals;
    if(iequals(ext, ".htm"))  return "text/html";
    if(iequals(ext, ".html")) return "text/html";
    if(iequals(ext, ".php"))  return "text/html";
    if(iequals(ext, ".css"))  return "text/css";
    if(iequals(ext, ".txt"))  return "text/plain";
    if(iequals(ext, ".sh"))   return "text/x-shellscript";
    if(iequals(ext, ".py"))   return "text/x-python";
    if(iequals(ext, ".jav"))  return "text/x-java-source";
    if(iequals(ext, ".java")) return "text/x-java-source";
    if(iequals(ext, ".pas"))  return "text/pascal";
    if(iequals(ext, ".js"))   return "application/javascript";
    if(iequals(ext, ".json")) return "application/json";
    if(iequals(ext, ".xml"))  return "application/xml";
    if(iequals(ext, ".class"))return "application/java";
    if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
    if(iequals(ext, ".flv"))  return "video/x-flv";
    if(iequals(ext, ".png"))  return "image/png";
    if(iequals(ext, ".jpe"))  return "image/jpeg";
    if(iequals(ext, ".jpeg")) return "image/jpeg";
    if(iequals(ext, ".jpg"))  return "image/jpeg";
    if(iequals(ext, ".gif"))  return "image/gif";
    if(iequals(ext, ".bmp"))  return "image/bmp";
    if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
    if(iequals(ext, ".tiff")) return "image/tiff";
    if(iequals(ext, ".tif"))  return "image/tiff";
    if(iequals(ext, ".svg"))  return "image/svg+xml";
    if(iequals(ext, ".svgz")) return "image/svg+xml";
    return mime_type_from_shebang(path).value_or(default_mime_type);
}

std::string path_cat(
    beast::string_view base,
    beast::string_view path)
{
    if(base.empty())
    {
        return std::string(path);
    }
    std::string result(base);

#if BOOST_MSVC
    constexpr char path_separator = '\\';
    if(result.back() == path_separator)
    {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
    for(auto& c : result)
    {
        if(c == '/')
        {
            c = path_separator;
        }
    }
#else
    constexpr char path_separator = '/';
    if(result.back() == path_separator)
    {
        result.resize(result.size() - 1);
    }
    result.append(path.data(), path.size());
#endif

    return result;
}


