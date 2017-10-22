#include <string>
#include <iostream>

#include "args.hxx"

#include "ntripcaster.h"


int main(int argc, char **argv)
{
    Config cfg{};

    args::ArgumentParser parser{ "ntripcaster", "Copyright 2017 JinXueyuan" };
    args::Group group(parser, "Must provied", args::Group::Validators::All);
    args::HelpFlag help{ parser, "help", "", {'h', "help"} };
    args::Positional<std::string> host(group, "host", "caster host");
    args::Positional<unsigned short> port(group, "port", "caster port");
    args::Positional<std::string> path(group, "path", "rinex path");

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cout << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cout << parser;
        return 1;
    }
    if (host) cfg.host = args::get(host);
    if (port) cfg.port = args::get(port);
    if (path) cfg.path = args::get(path);

    NtripCaster caster{ cfg };
    caster.start();
	return 0;
}